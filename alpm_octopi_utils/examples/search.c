
#include <alpm.h>
#include <alpm_list.h>
#include <alpm_octopi_utils.h>

int main()
{
	// create AlpmUtils instance
	AlpmUtils* alpm_utils = alpm_utils_new ("/etc/pacman.conf");

	// return a alpm_list of alpm_pkg, see alpm.h and alpm_list.h
	alpm_list_t* founds = alpm_utils_search_all_dbs (alpm_utils, "\\S");
	//alpm_list_t* founds = alpm_utils_get_all_pkgs(alpm_utils);
	//alpm_list_t* founds = alpm_utils_get_group_pkgs(alpm_utils, "base-devel");
	//alpm_list_t* founds = alpm_utils_get_updates(alpm_utils);
	//alpm_list_t* founds = alpm_utils_get_orphans(alpm_utils);
	//alpm_list_t* founds = alpm_utils_get_unrequired(alpm_utils);
	//alpm_list_t* founds = alpm_utils_get_foreign_pkgs(alpm_utils);

	
	//alpm_pkg_t* pkg = alpm_utils_get_installed_pkg(alpm_utils, "octopi");

	// display pkgs information
	
	alpm_list_t* i;
	const char* dbname;
	const char* installedVersion;
	const char* repoVersion;
	char* bInstalled;
	const char* pkgName;
	alpm_pkg_t* instPkg;

	for (i = founds; i; i = alpm_list_next(i)) {

		alpm_pkg_t* pkg = i->data;
		alpm_db_t* db = alpm_pkg_get_db(pkg);

		dbname = alpm_db_get_name(db);
		if (!strcmp(dbname, "local")) continue;

		repoVersion = alpm_pkg_get_version(pkg),

		pkgName = alpm_pkg_get_name(pkg),
		instPkg = (alpm_utils_get_installed_pkg(alpm_utils, pkgName));
		if (instPkg)
		{
		  bInstalled = "i";
	   	  installedVersion = alpm_pkg_get_version(instPkg);
		}
		else
		{
		  bInstalled = "n";
		  installedVersion = "same_version";
		}
		
		if (!strcmp(repoVersion, installedVersion))
		{
		  installedVersion = "same_version";	
		}

		printf("%s %s %s %s %s %d\n\t%s\n",
				bInstalled,
				dbname,
				pkgName,
				repoVersion,
				installedVersion,
				alpm_pkg_get_size(pkg),						
				alpm_pkg_get_desc(pkg));
	}

	// free
	alpm_utils_free (alpm_utils); // this will free all alpm_pkgs but not the alpm_list
	alpm_list_free (founds);
	
	return 0;
}
