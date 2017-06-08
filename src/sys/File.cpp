//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved


#include <stdio.h>
#include "sys/File.hpp"
using namespace sys;

File::File() {
	// TODO Auto-generated constructor stub
	m_fd = -1; //The file is not open
}

int File::remove(const char * name, link_transport_mdriver_t * driver){
#if defined __link
	return link_unlink(driver, name);
#else
	driver = 0;
	return ::remove(name);
#endif
}


int File::open(const char * name, int access, int perms){
	if( m_fd != -1 ){
		close(); //close and re-open
	}

#if defined __link
		m_fd = link_open(driver(), name, access, perms);
#else
	m_fd = ::open(name, access, perms);
#endif

	if( m_fd < 0 ){
		return -1;
	}
	return 0;
}

int File::create(const char * name, bool overwrite, int perms){
	int access = O_RDWR | O_CREAT;
	if( overwrite ){
		access |= O_TRUNC;
	}
	return open(name, access, perms);
}

ssize_t File::size() const {
	struct link_stat st;
#if defined __link
	if ( link_fstat(driver(), m_fd, &st) < 0 ){
		return -1;
	}
#else
	u32 loc;
	loc = lseek(m_fd, 0, SEEK_CUR);
	st.st_size = lseek(m_fd, 0, SEEK_END);
	lseek(m_fd, loc, SEEK_SET);
#endif
	return st.st_size;
}


/*! \brief Get file stat (pointer) */
int File::stat(const char * name, struct link_stat * st, link_transport_mdriver_t * driver){
#if defined __link
	return link_stat(driver, name, st);
#else
	return ::stat(name, (struct stat*)st);
#endif
}

/*! \brief Get file stat (reference) */
ssize_t File::size(const char * name, link_transport_mdriver_t * driver){
	struct link_stat st;
	if( stat(name, &st, driver) < 0 ){
		return (ssize_t)-1;
	}
	return st.st_size;
}
