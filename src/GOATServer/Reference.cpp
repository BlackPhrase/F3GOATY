/*
*******************************************************************************

Copyright (C) 2019-2020 SugarBombEngine Developers

This file is part of SugarBombEngine

SugarBombEngine is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

SugarBombEngine is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with SugarBombEngine. If not, see <http://www.gnu.org/licenses/>.

This file incorporates work originally covered by the MIT License. See
LICENSE-vaultmp file for details.

*******************************************************************************
*/
#include "Reference.hpp"
#include "API.hpp"
#include "Utils.hpp"
#include "Exterior.hpp"
#include "sqlite/sqlite3.h"

#include <cmath>
#include <algorithm>

using namespace DB;
using namespace Values;

std::unordered_map<unsigned int, Reference*> Reference::refs;
std::unordered_map<unsigned int, std::vector<Reference*>> Reference::cells;

Reference::Reference(const std::string& table, sqlite3_stmt* stmt)
{
	if (sqlite3_column_count(stmt) != 17)
		throw VaultException("Malformed input database (references): %s", table.c_str()).stacktrace();

	unsigned int dlc = static_cast<unsigned int>(sqlite3_column_int(stmt, 16));
	// if DLC enabled

	dlc <<= 24;

	constexpr double degrees = 180.0 / M_PI;

	type = Utils::str_replace(table, "refs_", "");
	editor = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
	refID = static_cast<unsigned int>(sqlite3_column_int(stmt, 1));
	baseID = static_cast<unsigned int>(sqlite3_column_int(stmt, 2));
	count = static_cast<unsigned int>(sqlite3_column_int(stmt, 3));
	health = sqlite3_column_double(stmt, 4);
	cell = static_cast<unsigned int>(sqlite3_column_int(stmt, 5));
	pos = make_tuple(sqlite3_column_double(stmt, 6), sqlite3_column_double(stmt, 7), sqlite3_column_double(stmt, 8));
	angle = make_tuple(sqlite3_column_double(stmt, 9) * degrees, sqlite3_column_double(stmt, 10) * degrees, sqlite3_column_double(stmt, 11) * degrees);
	flags = static_cast<unsigned int>(sqlite3_column_int(stmt, 12));
	lock = static_cast<unsigned int>(sqlite3_column_int(stmt, 13));

	if (lock == UINT_MAX) // -1
		lock = Lock_Impossible; // requires key
	else if (lock == UINT_MAX - 1) // -2
		lock = Lock_Unlocked; // unlocked

	key = static_cast<unsigned int>(sqlite3_column_int(stmt, 14));
	link = static_cast<unsigned int>(sqlite3_column_int(stmt, 15));

	if (cell & 0xFF000000)
	{
		cell &= 0x00FFFFFF;
		cell |= dlc;
	}

	auto exterior = Exterior::Lookup(cell);

	if (exterior)
	{
		auto match_exterior = DB::Exterior::Lookup(exterior->GetWorld(), get<0>(pos), get<1>(pos));

#ifdef VAULTMP_DEBUG
/*
		if (exterior->GetBase() != match_exterior->GetBase())
			debug.print("Error matching position with cell: ", hex, object->GetReference(), " relocating from ", dec, exterior->GetX(), ",", exterior->GetY(), " to ",  match_exterior->GetX(), ",", match_exterior->GetY());
*/
#endif
		cell = match_exterior->GetBase();
	}

	if (refID & 0xFF000000)
	{
		refID &= 0x00FFFFFF;
		refID |= dlc;
	}
	else
	{
		refs.erase(refID);
		cells[cell].erase(remove_if(cells[cell].begin(), cells[cell].end(), [this](const Reference* reference) { return reference->GetReference() == refID; }), cells[cell].end());
	}

	if (baseID & 0xFF000000)
	{
		baseID &= 0x00FFFFFF;
		baseID |= dlc;
	}

	if (key & 0xFF000000)
	{
		key &= 0x00FFFFFF;
		key |= dlc;
	}

	if (link & 0xFF000000)
	{
		link &= 0x00FFFFFF;
		link |= dlc;
	}

	refs.emplace(refID, this);
	cells[cell].emplace_back(this);
}

Expected<Reference*> Reference::Lookup(unsigned int refID)
{
	auto it = refs.find(refID);

	if (it != refs.end())
		return it->second;

	return VaultException("No reference with refID %08X found", refID);
}

std::vector<Reference*> Reference::Lookup(const std::string& type)
{
	std::vector<Reference*> data;

	for (const auto& ref : refs)
		if (!ref.second->GetType().compare(type))
			data.emplace_back(ref.second);

	return data;
}

const std::string& Reference::GetType() const
{
	return type;
}

const std::string& Reference::GetEditor() const
{
	return editor;
}

unsigned int Reference::GetReference() const
{
	return refID;
}

unsigned int Reference::GetBase() const
{
	return baseID;
}

unsigned int Reference::GetCount() const
{
	return count;
}

float Reference::GetHealth() const
{
	return health;
}

unsigned int Reference::GetCell() const
{
	return cell;
}

const std::tuple<float, float, float>& Reference::GetPos() const
{
	return pos;
}

const std::tuple<float, float, float>& Reference::GetAngle() const
{
	return angle;
}

unsigned int Reference::GetFlags() const
{
	return flags;
}

unsigned int Reference::GetLock() const
{
	return lock;
}

unsigned int Reference::GetKey() const
{
	return key;
}

unsigned int Reference::GetLink() const
{
	return link;
}
