/*! \file */ //Copyright 2011-2017 Tyler Gilbert; All Rights Reserved

#if defined __link
#include <sys/stat.h>
#endif

#include "fs/Stat.hpp"

using namespace fs;
#if defined __link
Stat::Stat(bool is_local) {
	memset(&m_stat, 0, sizeof(m_stat));
	m_is_local = is_local;
}
#else

Stat::Stat() {
	memset(&m_stat, 0, sizeof(m_stat));
}
#endif


bool Stat::is_directory() const {
#if defined __link
	if( m_is_local ){
		return (m_stat.st_mode & S_IFMT) == S_IFDIR;
	}
#endif
	return (m_stat.st_mode & Stat::FORMAT) == Stat::DIRECTORY;
}

bool Stat::is_file() const {
#if defined __link
	if( m_is_local ){
		return (m_stat.st_mode & S_IFMT) == S_IFREG;
	}
#endif
	return (m_stat.st_mode & Stat::FORMAT) == Stat::REGULAR;
}

bool Stat::is_device() const {
	return is_block_device() || is_character_device();

}

bool Stat::is_block_device() const {
#if defined __link
	if( m_is_local ){
		return (m_stat.st_mode & S_IFMT) == S_IFBLK;
	}
#endif
	return (m_stat.st_mode & (Stat::FORMAT)) == Stat::BLOCK;
}

bool Stat::is_character_device() const {
#if defined __link
	if( m_is_local ){
		return (m_stat.st_mode & S_IFMT) == S_IFCHR;
	}
#endif
	return (m_stat.st_mode & (Stat::FORMAT)) == Stat::CHARACTER;
}

bool Stat::is_socket() const {
#if defined __link
	if( m_is_local ){
#if !defined __win32
		return (m_stat.st_mode & S_IFMT) == S_IFSOCK;
#else
		return false;
#endif
	}
#endif
	return (m_stat.st_mode & Stat::FORMAT) == Stat::FILE_SOCKET;
}


u32 Stat::size() const {
	return m_stat.st_size;
}

bool Stat::is_executable() const {
	return false;
}

