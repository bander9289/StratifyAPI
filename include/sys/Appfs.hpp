/*! \file */ //Copyright 2011-2016 Tyler Gilbert; All Rights Reserved

#ifndef APPFS_HPP_
#define APPFS_HPP_

#include <sos/link.h>

namespace sys {

/*! \brief Application File System Class
 * \details This class provides an interface for creating data files in flash
 * memory.
 *
 * The following is a basic example of creating a data file in flash and then reading from it.
 *
 * \code
 * #include <sapi/sys.hpp>
 *
 * int main(int argc, char * argv[]){
 * 	char my_user_data[16];
 * 	//populate my_user_data as you please here
 * 	sprintf(my_user_data, "Hello World!");
 * 	Appfs::create("my_data", my_user_data, 16); //creates /app/flash/my_data as read only data
 * }
 * \endcode
 *
 * Now the file can be read just like any other file would be.  Of
 * course, reading files from /app/flash is very fast compared
 * to filesystems built on external chips.
 *
 * \code
 * #include <sapi/sys.hpp>
 *
 * int main(int argc, char * argv[]){
 * 	char my_user_data[16];
 * 	File f;
 * 	f.open("/app/flash/my_data", File::RDONLY); //only the file read only
 * 	f.read(my_user_data, 16); //read 16 bytes of user data
 * 	f.close();  //free resources
 * 	return 0;
 * }
 * \endcode
 *
 */
class Appfs {
public:

	/*! \details Creates a file in flash memory consisting
	 * of the data specified.
	 *
	 * @param name The name of the data file (no path info)
	 * @param buf A pointer to the data to be saved
	 * @param nbyte The number of bytes to save
	 * @param mount The mount path (default is /app)
	 * @return Zero on success or -1 with errno set accordingly
	 *
	 */
	static int create(const char * name,
			const void * buf,
			int nbyte,
			const char * mount = "/app",
			bool (*update)(void *, int, int) = 0,
			void * context = 0,
			link_transport_mdriver_t * driver = 0);


	/*! \details Returns the page size for writing data. */
	static int page_size(){ return APPFS_PAGE_SIZE; }

	/*! \details Gets the info associated with the file.
	 *
	 * @param path The path to the file (must be in the /app folder)
	 * @param info A reference to the destination info
	 * @return Zero on success
	 */
	static int get_info(const char * path, appfs_info_t & info);

	/*! \details Gets the application version.
	 *
	 * @param path The path to the file (must be in the /app folder)
	 * @return The BCD version of the file
	 *
	 * For example, the BCD representation of version "1.1" is 0x0101.
	 *
	 */
	static u16 get_version(const char * path);

#if !defined __link
	static int cleanup(bool data = false);
#endif
};

};

#endif /* APPFS_HPP_ */
