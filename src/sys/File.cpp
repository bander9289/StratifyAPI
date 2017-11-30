//Copyright 2011-2016 Tyler Gilbert; All Rights Reserved


#include <cstdio>
#include <cstring>
#include "sys/File.hpp"
#include "sys/Timer.hpp"
using namespace sys;

#if defined __link
File::File(link_transport_mdriver_t * d){
	// TODO Auto-generated constructor stub
	m_fd = -1; //The file is not open
	m_driver = d;
}
#else
File::File() {
	// TODO Auto-generated constructor stub
	m_fd = -1; //The file is not open
}
#endif

int File::open(const char * name, int flags){
	if( m_fd != -1 ){
		close(); //close and re-open
	}

#if defined __link
	m_fd = link_open(driver(), name, flags);
#else
	m_fd = ::open(name, flags);
#endif

	if( m_fd < 0 ){
		return -1;
	}
	return 0;
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

u32 File::size() const {
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

#if defined __link
int File::stat(const char * name, struct link_stat * st, link_transport_mdriver_t * driver){
	return link_stat(driver, name, st);
}
#else
int File::stat(const char * name, struct stat * st){
	return ::stat(name, st);
}

#endif

#if !defined __link
u32 File::size(const char * name){
	struct stat st;
	if( stat(name, &st) < 0 ){
		return (s32)-1;
	}
	return st.st_size;
}
#else
u32 File::size(const char * name, link_transport_mdriver_t * driver){
	struct link_stat st;
	if( stat(name, &st, driver) < 0 ){
		return (s32)-1;
	}
	return st.st_size;
}

#endif

int File::read(int loc, void * buf, int nbyte) const {
	if( seek(loc) < 0 ){
		return -1;
	}

	return read(buf, nbyte);
}

int File::write(int loc, const void * buf, int nbyte) const {
	if( seek(loc) < 0 ){
		return -1;
	}
	return write(buf, nbyte);
}


int File::readline(char * buf, int nbyte, int timeout, char term) const {
	int t;
	int bytes_recv;
	char c;
	t = 0;
	bytes_recv = 0;
	do {
		if( read(&c, 1) == 1 ){
			*buf = c;
			buf++;
			bytes_recv++;
			if( c == term ){
				return bytes_recv;
			}
		} else {
			t++;
			Timer::wait_msec(1);
		}
	} while( (bytes_recv < nbyte) && (t < timeout) );

	return bytes_recv;
}

int File::close(){
	int ret = 0;
	if( m_fd >= 0 ){
#if defined __link
		ret = link_close(driver(), m_fd);
#else
		ret = ::close(m_fd);
#endif
		m_fd = -1;
	}
	return ret;
}

int File::read(void * buf, int nbyte) const {
#if defined __link
	return link_read(driver(), m_fd, buf, nbyte);
#else
	return ::read(m_fd, buf, nbyte);
#endif
}

int File::write(const void * buf, int nbyte) const {
#if defined __link
	return link_write(driver(), m_fd, buf, nbyte);
#else
	return ::write(m_fd, buf, nbyte);
#endif
}

int File::seek(int loc, int whence) const {
#if defined __link
	return link_lseek(driver(), m_fd, loc, whence);
#else
	return ::lseek(m_fd, loc, whence);
#endif
}

int File::fileno() const {
	return m_fd;
}

int File::loc() const {
	return seek(0, CURRENT);
}

int File::flags() const{
	if( m_fd < 0 ){
		return -1;
	}
#if defined __link
	return -1;
#else
	return _global_impure_ptr->procmem_base->open_file[m_fd].flags;
#endif
}

char * File::gets(char * s, int n, char term) const {
	int t;
	int ret;
	int i;
	char buffer[GETS_BUFFER_SIZE];

	if( n < 1 ){
		return 0;
	}

	s[0] = 0;
	t = 0;
	do {
		ret = read(buffer, GETS_BUFFER_SIZE);
		for(i=0; i < ret; i++){
			s[t] = buffer[i];
			s[t+1] = 0;
			if( s[t] == '\n'){
				break;
			}
			if( t == (n-1) ){
				break;
			}
			t++;
		}
	} while( (t < (n-1)) && (s[t] != term) && (ret > 0) );

	if( t == 0 ){
		return 0;
	}

	if( ret > 0 ){
		seek(i - ret + 1, CURRENT);
	}

	return s;

}

const char * File::name(const char * path){
	int len;
	int i;
	len = strnlen(path, LINK_PATH_MAX);
	for(i = len; i >= 0; i--){
		if( path[i] == '/' ){
			return &(path[i+1]);
		}
	}
	return 0;
}

const char * File::suffix(const char * path){
	int len;
	int i;
	len = strnlen(path, LINK_PATH_MAX);
	for(i = len; i >= 0; i--){
		if( path[i] == '.' ){
			return &(path[i+1]);
		}
	}
	return 0;
}


int File::ioctl(int req, void * arg) const {
#if defined __link
	return link_ioctl(driver(), m_fd, req, arg);
#else
	return ::ioctl(m_fd, req, arg);
#endif
}

