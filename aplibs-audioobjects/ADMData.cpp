
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

#include <algorithm>

#define DEBUG_LEVEL 2
#include "ADMData.h"
#include "RIFFChunk_Definitions.h"

using namespace std;

vector<ADMData::PROVIDER> ADMData::providerlist;

ADMData::ADMData()
{
}

ADMData::~ADMData()
{
	Delete();
}

void ADMData::Delete()
{
	ADMOBJECTS_IT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		delete it->second;
	}

	admobjects.clear();
	tracklist.clear();
}

bool ADMData::SetChna(const uint8_t *data)
{
	const CHNA_CHUNK& chna = *(const CHNA_CHUNK *)data;
	bool success = true;

	uint16_t i;
	for (i = 0; i < chna.UIDCount; i++) {
		ADMAudioTrack *track;
		string id;

		id.assign(chna.UIDs[i].UID, sizeof(chna.UIDs[i].UID));

		if ((track = dynamic_cast<ADMAudioTrack *>(Create(ADMAudioTrack::Type, id, ""))) != NULL) {
			ADMVALUE value;

			value.attr = false;

			track->SetTrackNum(chna.UIDs[i].TrackNum);

			value.name = ADMAudioTrackFormat::Reference;
			value.value.assign(chna.UIDs[i].TrackRef, sizeof(chna.UIDs[i].TrackRef));
			track->AddValue(value);
			
			value.name = ADMAudioPackFormat::Reference;
			value.value.assign(chna.UIDs[i].PackRef, sizeof(chna.UIDs[i].PackRef));
			track->AddValue(value);

			track->SetValues();
		}
		else ERROR("Failed to create AudioTrack for UID %u", i);
	}

	return success;
}

bool ADMData::SetAxml(const uint8_t *data, uint_t length)
{
	string str;

	str.assign((const char *)data, length);

	return SetAxml(str);
}

bool ADMData::SetAxml(const string& data)
{
	bool success = false;

	DEBUG4(("XML: %s", data.c_str()));

	if (TranslateXML(data)) {
		success = true;
	}

	return success;
}

bool ADMData::Set(const uint8_t *chna, const uint8_t *axml, uint_t axmllength)
{
	bool success = false;

	if (SetChna(chna) && SetAxml(axml, axmllength)) {
		SortTracks();
		ConnectReferences();
		UpdateLimits();

		success = true;
	}

	return success;
}

uint8_t *ADMData::GetChna(uint32_t& len) const
{
	CHNA_CHUNK *p = NULL;

	len = sizeof(*p) + tracklist.size() * sizeof(p->UIDs[0]);
	if ((p = (CHNA_CHUNK *)calloc(1, len)) != NULL) {
		uint_t i;

		p->TrackCount = tracklist.size();
		p->UIDCount   = tracklist.size();
		
		for (i = 0; i < p->UIDCount; i++) {
			const ADMAudioTrack *track = tracklist[i];

			p->UIDs[i].TrackNum = track->GetTrackNum();
			strncpy(p->UIDs[i].UID, track->GetID().c_str(), sizeof(p->UIDs[i].UID));

			const ADMAudioTrackFormat *trackref = track->GetTrackFormatRefs().front();
			if (trackref) strncpy(p->UIDs[i].TrackRef, trackref->GetID().c_str(), sizeof(p->UIDs[i].TrackRef));

			const ADMAudioPackFormat *packref = track->GetPackFormatRefs().front();
			if (packref) strncpy(p->UIDs[i].PackRef, packref->GetID().c_str(), sizeof(p->UIDs[i].PackRef));

			DEBUG2(("Track %u/%u: Index %u UID '%s' TrackFormatRef '%s' PackFormatRef '%s'",
					i + 1, p->UIDCount,
					track->GetTrackNum(),
					track->GetID().c_str(),
					trackref ? trackref->GetID().c_str() : "<none>",
					packref  ? packref->GetID().c_str()  : "<none>"));

		}
	}

	return (uint8_t *)p;
}

string ADMData::GetAxml(const string& indent, const string& eol, uint_t ind_level) const
{
	string str;

	Printf(str,
		   "%s<?xml version=\"1.0\" encoding=\"UTF-8\"?>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str());

	Printf(str,
		   "%s<ebuCoreMain xmlns:dc=\"http://purl.org/dc/elements/1.1/\" xmlns=\"urn:ebu:metadata-schema:ebuCore_2014\" xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" schema=\"EBU_CORE_20140201.xsd\" xml:lang=\"en\">%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

	GenerateXML(str, indent, eol, ind_level);

	ind_level--;
	Printf(str,
		   "%s</ebuCoreMain>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str());

	return str;
}

ADMData *ADMData::Create()
{
	ADMData *data = NULL;
	uint_t i;

	for (i = 0; i < providerlist.size(); i++) {
		const PROVIDER& provider = providerlist[i];

		if ((data = (*provider.fn)(provider.context)) != NULL) break;
	}

	return data;
}

void ADMData::RegisterProvider(CREATOR fn, void *context)
{
	PROVIDER provider = {
		.fn      = fn,
		.context = context,
	};

	providerlist.push_back(provider);
}

void ADMData::Register(ADMObject *obj, const string& type)
{
	string uuid = type + "/" + obj->GetID();

	admobjects[uuid] = obj;

	{
		const ADMAudioTrack *track;
		if ((track = dynamic_cast<const ADMAudioTrack *>(obj)) != NULL) {
			tracklist.push_back(track);
		}
	}
}

bool ADMData::ValidType(const string& type) const
{
	return ((type == ADMAudioProgramme::Type) ||
			(type == ADMAudioContent::Type) ||
			(type == ADMAudioObject::Type) ||
			(type == ADMAudioPackFormat::Type) ||
			(type == ADMAudioBlockFormat::Type) ||
			(type == ADMAudioChannelFormat::Type) ||
			(type == ADMAudioStreamFormat::Type) ||
			(type == ADMAudioTrackFormat::Type) ||
			(type == ADMAudioTrack::Type));
}

ADMObject *ADMData::Create(const string& type, const string& id, const string& name)
{
	ADMObject *obj = NULL;

	if (ValidType(type)) {
		ADMOBJECTS_MAP::const_iterator it;
		string uuid = type + "/" + id;

		if ((it = admobjects.find(uuid)) == admobjects.end()) {
			if		(type == ADMAudioProgramme::Type) 	  obj = new ADMAudioProgramme(*this, id, name);
			else if (type == ADMAudioContent::Type)   	  obj = new ADMAudioContent(*this, id, name);
			else if (type == ADMAudioObject::Type)		  obj = new ADMAudioObject(*this, id, name);
			else if (type == ADMAudioPackFormat::Type)    obj = new ADMAudioPackFormat(*this, id, name);
			else if (type == ADMAudioBlockFormat::Type)   obj = new ADMAudioBlockFormat(*this, id, name);
			else if (type == ADMAudioChannelFormat::Type) obj = new ADMAudioChannelFormat(*this, id, name);
			else if (type == ADMAudioStreamFormat::Type)  obj = new ADMAudioStreamFormat(*this, id, name);
			else if (type == ADMAudioTrackFormat::Type)   obj = new ADMAudioTrackFormat(*this, id, name);
			else if (type == ADMAudioTrack::Type)		  obj = new ADMAudioTrack(*this, id, name);
		}
		else obj = it->second;
	}

	return obj;
}

ADMObject *ADMData::Parse(const string& type, void *userdata)
{
	ADMHEADER header;
	ADMObject *obj;

	ParseHeader(header, type, userdata);
	
	if ((obj = Create(type, header.id, header.name)) != NULL) {
		ParseValues(obj, type, userdata);
		PostParse(obj, type, userdata);

		obj->SetValues();
	}

	return obj;
}

ADMObject *ADMData::GetReference(const ADMVALUE& value)
{
	ADMObject *obj = NULL;
	ADMOBJECTS_CIT it;
	string uuid = value.name, cmp;

	cmp = "UIDRef";
	if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0)) {
		uuid = uuid.substr(0, uuid.size() - 3);
	}
	else {
		cmp = "IDRef";

		if ((uuid.size() >= cmp.size()) && (uuid.compare(uuid.size() - cmp.size(), cmp.size(), cmp) == 0)) {
			uuid = uuid.substr(0, uuid.size() - cmp.size());
		}
	}

	uuid += "/" + value.value;

	if ((it = admobjects.find(uuid)) != admobjects.end()) obj = it->second;

	if (!obj) DEBUG2(("Failed to find reference '%s'", uuid.c_str()));

	return obj;
}

void ADMData::SortTracks()
{
	vector<const ADMAudioTrack *>::const_iterator it;

	sort(tracklist.begin(), tracklist.end(), ADMAudioTrack::Compare);

	DEBUG4(("%lu tracks:", tracklist.size()));
	for (it = tracklist.begin(); it != tracklist.end(); ++it) {
		DEBUG4(("%u: %s", (*it)->GetTrackNum(), (*it)->ToString().c_str()));
	}
}

void ADMData::ConnectReferences()
{
	ADMOBJECTS_IT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		it->second->SetReferences();
	}
}

void ADMData::UpdateLimits()
{
	ADMOBJECTS_IT  it;
	ADMAudioObject *obj;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		if ((obj = dynamic_cast<ADMAudioObject *>(it->second)) != NULL) {
			obj->UpdateLimits();
		}
	}
}

void ADMData::GetADMList(const string& type, vector<const ADMObject *>& list) const
{
	ADMOBJECTS_CIT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		const ADMObject *obj = it->second;

		if (obj->GetType() == type) {
			
			list.push_back(obj);
		}
	}
}

const ADMObject *ADMData::GetObjectByID(const string& id, const string& type) const
{
	ADMOBJECTS_CIT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		const ADMObject *obj = it->second;

		if (((type == "") || (obj->GetType() == type)) && (obj->GetID() == id)) return obj;
	}

	return NULL;
}

const ADMObject *ADMData::GetObjectByName(const string& name, const string& type) const
{
	ADMOBJECTS_CIT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		const ADMObject *obj = it->second;

		if (((type == "") || (obj->GetType() == type)) && (obj->GetName() == name)) return obj;
	}

	return NULL;
}

string ADMData::FormatString(const char *fmt, ...)
{
	string  str;
	va_list ap;

	va_start(ap, fmt);

	char *buf = NULL;
	if (vasprintf(&buf, fmt, ap) > 0) {
		str = buf;
		free(buf);
	}

	va_end(ap);

	return str;
}

void ADMData::Dump(string& str, const string& indent, const string& eol, uint_t level) const
{
	ADMOBJECTS_CIT it;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		if (it->second->GetType() == ADMAudioProgramme::Type) {
			it->second->Dump(str, indent, eol, level);
		}
	}
}

void ADMData::GenerateXML(string& str, const string& indent, const string& eol, uint_t ind_level) const
{
	ADMOBJECTS_CIT it;

	Printf(str,
		   "%s<coreMetadata>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

	Printf(str,
		   "%s<format>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

	Printf(str,
		   "%s<audioFormatExtended>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str()); ind_level++;

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		const ADMObject *obj = it->second;

		if (obj->GetType() == ADMAudioProgramme::Type) {
			obj->GenerateXML(str, indent, eol, ind_level);
		}
	}

	ind_level--;
	Printf(str,
		   "%s</audioFormatExtended>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str());

	ind_level--;
	Printf(str,
		   "%s</format>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str());
	
	ind_level--;
	Printf(str,
		   "%s</coreMetadata>%s",
		   CreateIndent(indent, ind_level).c_str(), eol.c_str());
}

void ADMData::CreateCursors(std::vector<PositionCursor *>& list, uint_t channel, uint_t nchannels) const
{
	uint_t i;

	channel   = MIN(channel,   tracklist.size() - 1);
	nchannels = MIN(nchannels, tracklist.size() - channel);

	for (i = 0; i < nchannels; i++) {
		list.push_back(new ADMTrackCursor(tracklist[channel + i]));
	}
}

void ADMData::Serialize(uint8_t *dst, uint_t& len) const
{
	ADMOBJECTS_CIT it;
	uint_t len0   = len;
	uint_t sublen = 0;

	if (dst) Serialize(NULL, sublen);

	ADMObject::SerializeData(dst, len, ADMObject::SerialDataType_ADMHeader, sublen);
	ADMObject::SerializeData(dst, len, (uint32_t)admobjects.size());
	ADMObject::SerializeSync(dst, len, len0);

	for (it = admobjects.begin(); it != admobjects.end(); ++it) {
		it->second->Serialize(dst, len);
	}

	ADMObject::SerializeObjectCRC(dst, len, len0);
}

