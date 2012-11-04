/*

Copyright (c) 2012, Arvid Norberg, Magnus Jonsson
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in
      the documentation and/or other materials provided with the distribution.
    * Neither the name of the author nor the names of its
      contributors may be used to endorse or promote products derived
      from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.

*/

#include "save_settings.hpp"

#include <boost/bind.hpp>
#include "libtorrent/add_torrent_params.hpp"
#include "libtorrent/session.hpp"
#include "libtorrent/error_code.hpp"
#include "libtorrent/entry.hpp"
#include "libtorrent/bencode.hpp"
#include "libtorrent/lazy_entry.hpp"
#include "libtorrent/file.hpp"

namespace libtorrent
{

int save_file(std::string const& filename, std::vector<char>& v, error_code& ec)
{
	file f;
	if (!f.open(filename, file::write_only, ec)) return -1;
	if (ec) return -1;
	file::iovec_t b = {&v[0], v.size()};
	size_type written = f.writev(0, &b, 1, ec);
	if (written != int(v.size())) return -3;
	if (ec) return -3;
	return 0;
}

save_settings::save_settings(session& s, std::string const& settings_file)
	: m_ses(s)
	, m_settings_file(settings_file)
{}

save_settings::~save_settings() {}

void save_settings::save(error_code& ec) const
{
	// TODO: back-up current settings file as .bak before saving the new one
	entry sett;
	m_ses.save_state(sett);

	for (std::map<std::string, int>::const_iterator i = m_ints.begin()
		, end(m_ints.end()); i != end; ++i)
	{
		sett[i->first] = i->second;
	}

	for (std::map<std::string, std::string>::const_iterator i = m_strings.begin()
		, end(m_strings.end()); i != end; ++i)
	{
		sett[i->first] = i->second;
	}
	std::vector<char> buf;
	bencode(std::back_inserter(buf), sett);
	save_file(m_settings_file, buf, ec);
}

void save_settings::load(error_code& ec)
{
	// TODO: if the file doesn't exist or is invalid, try the .bak version
	std::vector<char> buf;
	if (load_file(m_settings_file, buf, ec) < 0)
		return;

	lazy_entry sett;
	if (lazy_bdecode(&buf[0], &buf[0] + buf.size(), sett, ec) != 0)
		return;

	m_ses.load_state(sett);

	// TODO: load the int and string keys
//	for ()
}

void save_settings::set_int(char const* key, int val)
{
	if (val == 0)
	{
		std::map<std::string, int>::iterator i = m_ints.find(key);
		if (i != m_ints.end()) m_ints.erase(i);
		return;
	}
	m_ints[key] = val;
}

void save_settings::set_str(char const* key, std::string val)
{
	if (val.empty())
	{
		std::map<std::string, std::string>::iterator i = m_strings.find(key);
		if (i != m_strings.end()) m_strings.erase(i);
		return;
	}
	m_strings[key] = val;
}

int save_settings::get_int(char const* key) const
{
	std::map<std::string, int>::const_iterator i = m_ints.find(key);
	if (i == m_ints.end()) return 0;
	return i->second;
}

std::string save_settings::get_str(char const* key) const
{
	std::map<std::string, std::string>::const_iterator i = m_strings.find(key);
	if (i == m_strings.end()) return "";
	return i->second;
}

}

