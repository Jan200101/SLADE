
#include "Main.h"
#include "Executables.h"
#include "ArchiveManager.h"
#include "Parser.h"

namespace Executables
{
	vector<game_exe_t>	game_exes;
	vector<key_value_t>	exe_paths;
}

Executables::game_exe_t* Executables::getGameExe(string id)
{
	for (unsigned a = 0; a < game_exes.size(); a++)
	{
		if (game_exes[a].id == id)
			return &(game_exes[a]);
	}

	return NULL;
}

Executables::game_exe_t* Executables::getGameExe(unsigned index)
{
	if (index < game_exes.size())
		return &(game_exes[index]);
	else
		return NULL;
}

unsigned Executables::nGameExes()
{
	return game_exes.size();
}

void Executables::setExePath(string id, string path)
{
	exe_paths.push_back(key_value_t(id, path));
}

string Executables::writePaths()
{
	string ret;
	for (unsigned a = 0; a < game_exes.size(); a++)
		ret += S_FMT("\t%s \"%s\"\n", CHR(game_exes[a].id), CHR(game_exes[a].path));
	return ret;
}

string Executables::writeExecutables()
{
	string ret = "executables\n{\n";

	// Go through game exes
	for (unsigned a = 0; a < game_exes.size(); a++)
	{
		// ID
		ret += S_FMT("\t%s\n\t{\n", CHR(game_exes[a].id));

		// Name
		ret += S_FMT("\t\tname = \"%s\";\n", CHR(game_exes[a].name));

		// Exe name
		ret += S_FMT("\t\texe_name = \"%s\";\n\n", CHR(game_exes[a].exe_name));

		// Configs
		for (unsigned b = 0; b < game_exes[a].configs.size(); b++)
			ret += S_FMT("\t\tconfig \"%s\" = \"%s\";\n", CHR(game_exes[a].configs[b].key), CHR(game_exes[a].configs[b].value));

		ret += "\t}\n\n";
	}

	ret += "}";

	return ret;
}

void Executables::init()
{
	// Load from pk3
	Archive* res_archive = theArchiveManager->programResourceArchive();
	ArchiveEntry* entry = res_archive->entryAtPath("config/executables.cfg");
	if (!entry)
		return;

	// Parse base executables config
	Parser p;
	p.parseText(entry->getMCData(), "slade.pk3 - executables.cfg");
	parse(&p);

	// Parse user executables config
	Parser p2;
	MemChunk mc;
	if (mc.importFile(appPath("executables.cfg", DIR_USER)))
	{
		p2.parseText(mc, "user execuatbles.cfg");
		parse(&p2);
	}
}

void Executables::parse(Parser* p)
{
	ParseTreeNode* n = (ParseTreeNode*)p->parseTreeRoot()->getChild("executables");
	for (unsigned a = 0; a < n->nChildren(); a++)
	{
		// Get game_exe_t being parsed
		ParseTreeNode* exe_node = (ParseTreeNode*)n->getChild(a);
		game_exe_t* exe = getGameExe(exe_node->getName().Lower());
		if (!exe)
		{
			// Create if new
			game_exe_t nexe;
			game_exes.push_back(nexe);
			exe = &(game_exes.back());
		}

		exe->id = exe_node->getName();
		for (unsigned b = 0; b < exe_node->nChildren(); b++)
		{
			ParseTreeNode* prop = (ParseTreeNode*)exe_node->getChild(b);
			string prop_name = prop->getName().Lower();

			// Config
			if (prop->getType().Lower() == "config")
			{
				// Update if exists
				bool found = false;
				for (unsigned c = 0; c < exe->configs.size(); c++)
				{
					if (exe->configs[c].key == prop->getName())
					{
						exe->configs[c].value = prop->getStringValue();
						found = true;
					}
				}

				// Create if new
				if (!found)
					exe->configs.push_back(key_value_t(prop->getName(), prop->getStringValue()));
			}

			// Name
			else if (prop_name == "name")
				exe->name = prop->getStringValue();

			// Executable name
			else if (prop_name == "exe_name")
				exe->exe_name = prop->getStringValue();
		}

		// Set path if loaded
		for (unsigned p = 0; p < exe_paths.size(); p++)
		{
			if (exe_paths[p].key == exe->id)
				exe->path = exe_paths[p].value;
		}
	}
}

void Executables::addGameExe(string name)
{
	game_exe_t game;
	game.name = name;
	
	name.Replace(" ", "_");
	name.MakeLower();
	game.id = name;

	game_exes.push_back(game);
}
