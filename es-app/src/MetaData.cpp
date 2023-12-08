#include "MetaData.h"

#include "utils/FileSystemUtil.h"
#include "Log.h"
#include <pugixml.hpp>
#include "Locale.h"

MetaDataDecl gameDecls[] = {
	// key,         type,                   default,            statistic,  name in GuiMetaDataEd,  prompt in GuiMetaDataEd
	{"name",        MD_STRING,              "",                 false,      N_("name"),                 N_("enter game name")},
	{"sortname",    MD_STRING,              "",                 false,      N_("sortname"),             N_("enter game sort name")},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      N_("description"),          N_("enter description")},
	{"image",       MD_PATH,                "",                 false,      N_("image"),                N_("enter path to image")},
	{"video",       MD_PATH     ,           "",                 false,      N_("video"),                N_("enter path to video")},
	{"marquee",     MD_PATH,                "",                 false,      N_("marquee"),              N_("enter path to marquee")},
	{"thumbnail",   MD_PATH,                "",                 false,      N_("thumbnail"),            N_("enter path to thumbnail")},
	{"rating",      MD_RATING,              "0.000000",         false,      N_("rating"),               N_("enter rating")},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      N_("release date"),         N_("enter release date")},
	{"developer",   MD_STRING,              "unknown",          false,      N_("developer"),            N_("enter game developer")},
	{"publisher",   MD_STRING,              "unknown",          false,      N_("publisher"),            N_("enter game publisher")},
	{"genre",       MD_STRING,              "unknown",          false,      N_("genre"),                N_("enter game genre")},
	{"players",     MD_INT,                 "1",                false,      N_("players"),              N_("enter number of players")},
	{"favorite",    MD_BOOL,                "false",            false,      N_("favorite"),             N_("enter favorite off/on")},
	{"hidden",      MD_BOOL,                "false",            false,      N_("hidden"),               N_("enter hidden off/on") },
	{"kidgame",     MD_BOOL,                "false",            false,      N_("kidgame"),              N_("enter kidgame off/on") },
	{"playcount",   MD_INT,                 "0",                true,       N_("play count"),           N_("enter number of times played")},
	{"lastplayed",  MD_TIME,                "0",                true,       N_("last played"),          N_("enter last played date")}
};
const std::vector<MetaDataDecl> gameMDD(gameDecls, gameDecls + sizeof(gameDecls) / sizeof(gameDecls[0]));

MetaDataDecl folderDecls[] = {
	{"name",        MD_STRING,              "",                 false,      N_("name"),                 N_("enter game name")},
	{"sortname",    MD_STRING,              "",                 false,      N_("sortname"),             N_("enter game sort name")},
	{"desc",        MD_MULTILINE_STRING,    "",                 false,      N_("description"),          N_("enter description")},
	{"image",       MD_PATH,                "",                 false,      N_("image"),                N_("enter path to image")},
	{"thumbnail",   MD_PATH,                "",                 false,      N_("thumbnail"),            N_("enter path to thumbnail")},
	{"video",       MD_PATH,                "",                 false,      N_("video"),                N_("enter path to video")},
	{"marquee",     MD_PATH,                "",                 false,      N_("marquee"),              N_("enter path to marquee")},
	{"rating",      MD_RATING,              "0.000000",         false,      N_("rating"),               N_("enter rating")},
	{"releasedate", MD_DATE,                "not-a-date-time",  false,      N_("release date"),         N_("enter release date")},
	{"developer",   MD_STRING,              "unknown",          false,      N_("developer"),            N_("enter game developer")},
	{"publisher",   MD_STRING,              "unknown",          false,      N_("publisher"),            N_("enter game publisher")},
	{"genre",       MD_STRING,              "unknown",          false,      N_("genre"),                N_("enter game genre")},
	{"players",     MD_INT,                 "1",                false,      N_("players"),              N_("enter number of players")}
};
const std::vector<MetaDataDecl> folderMDD(folderDecls, folderDecls + sizeof(folderDecls) / sizeof(folderDecls[0]));

const std::vector<MetaDataDecl>& getMDDByType(MetaDataListType type)
{
	switch(type)
	{
	case GAME_METADATA:
		return gameMDD;
	case FOLDER_METADATA:
		return folderMDD;
	}

	LOG(LogError) << "Invalid MDD type";
	return gameMDD;
}



MetaDataList::MetaDataList(MetaDataListType type)
	: mType(type), mWasChanged(false)
{
	const std::vector<MetaDataDecl>& mdd = getMDD();
	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
		set(iter->key, iter->defaultValue);
}


MetaDataList MetaDataList::createFromXML(MetaDataListType type, pugi::xml_node& node, const std::string& relativeTo)
{
	MetaDataList mdl(type);

	const std::vector<MetaDataDecl>& mdd = mdl.getMDD();

	for(auto iter = mdd.cbegin(); iter != mdd.cend(); iter++)
	{
		pugi::xml_node md = node.child(iter->key.c_str());
		if(md)
		{
			// if it's a path, resolve relative paths
			std::string value = md.text().get();
			if (iter->type == MD_PATH)
			{
				value = Utils::FileSystem::resolveRelativePath(value, relativeTo, true, true);
			}
			mdl.set(iter->key, value);
		}else{
			mdl.set(iter->key, iter->defaultValue);
		}
	}

	return mdl;
}

void MetaDataList::appendToXML(pugi::xml_node& parent, bool ignoreDefaults, const std::string& relativeTo) const
{
	const std::vector<MetaDataDecl>& mdd = getMDD();

	for(auto mddIter = mdd.cbegin(); mddIter != mdd.cend(); mddIter++)
	{
		auto mapIter = mMap.find(mddIter->key);
		if(mapIter != mMap.cend())
		{
			// we have this value!
			// if it's just the default (and we ignore defaults), don't write it
			if(ignoreDefaults && mapIter->second == mddIter->defaultValue)
				continue;

			// try and make paths relative if we can
			std::string value = mapIter->second;
			if (mddIter->type == MD_PATH)
				value = Utils::FileSystem::createRelativePath(value, relativeTo, true, true);

			parent.append_child(mapIter->first.c_str()).text().set(value.c_str());
		}
	}
}

void MetaDataList::set(const std::string& key, const std::string& value)
{
	mMap[key] = value;
	mWasChanged = true;
}

const std::string& MetaDataList::get(const std::string& key) const
{
	return mMap.at(key);
}

int MetaDataList::getInt(const std::string& key) const
{
	return atoi(get(key).c_str());
}

float MetaDataList::getFloat(const std::string& key) const
{
	return (float)atof(get(key).c_str());
}

bool MetaDataList::wasChanged() const
{
	return mWasChanged;
}

void MetaDataList::resetChangedFlag()
{
	mWasChanged = false;
}
