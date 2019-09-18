/*! \file */ //Copyright 2011-2018 Tyler Gilbert; All Rights Reserved

#ifndef SAPI_SYS_DIR_HPP_
#define SAPI_SYS_DIR_HPP_

#ifdef __link
#include <sos/link.h>

#if defined __win32
#include <winsock2.h>
#include <windows.h>
#if !defined FALSE
#define FALSE 0
#endif
#if !defined TRUE
#define TRUE 1
#endif
#include "dirent_windows.h"
#undef ERROR
#else
#include <dirent.h>
#endif


#else
#include <dirent.h>
#endif

#include "../api/FsObject.hpp"
#include "../var/Vector.hpp"
#include "../var/String.hpp"
#include "../var/ConstString.hpp"
#include "../arg/Argument.hpp"
#include "Stat.hpp"

namespace fs {

/*! \brief Dir class
 *
 * \todo Add some examples
 *
 *
 */
class Dir : public api::FsWorkObject {
public:
	/*! \details Constructs a Dir object. */
#if defined __link
	Dir(arg::LinkDriver driver = arg::LinkDriver(0));
	static int create(
			const arg::DestinationDirectoryPath & path,
			const Permissions & permissions = Permissions(0777),
			arg::LinkDriver driver = arg::LinkDriver(0)
			);

	static int create(
			const arg::DestinationDirectoryPath & path,
			const Permissions & permissions,
			const arg::IsRecursive is_recursive,
			arg::LinkDriver driver = arg::LinkDriver(0)
			);

	static bool exists(
			const arg::SourceDirectoryPath & path,
			arg::LinkDriver driver = arg::LinkDriver(0)
			);

	static var::Vector<var::String> read_list(
			const arg::SourceDirectoryPath & path,
			arg::LinkDriver driver = arg::LinkDriver(0)
			);

#else
	Dir();

	/*! \details Returns true if the directory exists. */
	static int create(
			const arg::DestinationDirectoryPath & path,
			const Permissions & permissions = Permissions(0777)
			);
	static int create(
			const arg::DestinationDirectoryPath & path,
			const Permissions & permissions,
			const arg::IsRecursive is_recursive
			);
	/*! \details Returns true if the directory exists. */
	static bool exists(
			const arg::SourceDirectoryPath & path
			);
	static var::Vector<var::String> read_list(
			const arg::SourceDirectoryPath & path
			);
#endif

	static int copy(
			const arg::SourceDirectoryPath source_path,
			const arg::DestinationDirectoryPath destination_path
		#if defined __link
			, arg::SourceLinkDriver source_driver = arg::SourceLinkDriver(0),
			arg::DestinationLinkDriver destination_driver = arg::DestinationLinkDriver(0)
		#endif
			);

	/*! \details Destructs the object.
	 *
	 * If the object has a directory that is
	 * currently open, the directory will
	 * be closed upon destruction.
	 *
	 */
	~Dir();

	/*! \details Opens a directory. */
	int open(const arg::SourceDirectoryPath & name);
	/*! \details Closes the directory.
	 *
	 * If this method is not called explicitly before
	 * the object is destroyed, it will be called
	 * during destruction. See ~Dir().
	 *
	 */
	int close();

	/*! \details Returns true if the directory is open. */
	bool is_open() const { return m_dirp != 0; }

	/*! \details Returns a pointer to the next entry or 0 if no more entries exist.
	 */
	const char * read();

	/*! \details Removes a directory.
	 *
	 * @return Zero on success or -1 for an error
	 */
#if !defined __link
	static int remove(
			const arg::SourceDirectoryPath path,
			const arg::IsRecursive recursive = arg::IsRecursive(false));
#else
	static int remove(const arg::SourceDirectoryPath path,
							const arg::IsRecursive recursive,
							arg::LinkDriver driver = arg::LinkDriver(0)
			);
#endif

	/*! \details Gets the next entry and writes the full path of the entry to the given string.
	 *
	 * @param path_dest The var::String that will hold the full path of the next entry.
	 * @return True if an entry was read or false for an error or no more entries
	 */
	bool get_entry(var::String & path_dest);

	var::String get_entry();

	/*! \details Returns a list of all
	 * the entries in the directory.
	 *
	 *
	 * ```
	 * #include <sapi/fs.hpp>
	 * #include <sapi/var.hpp>
	 *
	 *
	 * Dir d;
	 *
	 * d.open(arg::SourceDirectoryPath("/home");
	 * Vector<String> list = d.read_list();
	 * d.close();
	 *
	 * for(u32 i=0; i < list.count(); i++){
	 *   printf("Entry is %s\n", list.at(i).cstring());
	 * }
	 *
	 * ```
	 *
	 *
	 *
	 */
	var::Vector<var::String> read_list();


	/*! \details Returns a pointer (const) to the name of the most recently read entry. */
	const char * name(){
		return m_entry.d_name;
	}

	/*! \details Returns a pointer (editable) to the name of the most recently read entry. */
	char * data(){
		return m_entry.d_name;
	}

	/*! \details Returns the serial number of the most recently read entry. */
	int ino(){
		return m_entry.d_ino;
	}

#ifndef __link
	/*! \details Returns the directory handle pointer. */
	DIR * dirp(){ return m_dirp; }
	/*! \details Counts the total number of entries in the directory. */
	int count();
	/*! \details Rewinds the directory pointer. */
	void rewind(){
		if( m_dirp ) {
			rewinddir(m_dirp);
		}
	}
	/*! \details Seeks to a location in the directory.
	 *
	 * Each entry in the directory occupies 1 location
	 * space. The first entry is at location 0.
	 *
	 *
	 */
	void seek(arg::Location location){
		if( m_dirp ) {
			seekdir(m_dirp, location.argument());
		}
	}

	/*! \details Returns the current location in the directory.
	 *
	 *
	 */
	inline long tell(){
		if( m_dirp ){
			return telldir(m_dirp);
		}
		return 0;
	}
#else
	void set_driver(link_transport_mdriver_t * d){ m_driver = d; }
#endif

private:
#ifdef __link
	int m_dirp;
	struct link_dirent m_entry;
	link_transport_mdriver_t * m_driver;
	link_transport_mdriver_t * driver(){ return m_driver; }

	DIR * m_dirp_local;
	struct dirent m_entry_local;

#else
	DIR * m_dirp;
	struct dirent m_entry;
#endif

	var::String m_path;


};

}

#endif /* SAPI_SYS_DIR_HPP_ */