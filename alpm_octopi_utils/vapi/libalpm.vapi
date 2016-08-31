/*
 *  Vala bindings for libalpm
 *
 *  Copyright (C) 2014-2016 Guillaume Benoit <guillaume@manjaro.org>
 *  Copyright (c) 2011 Rémy Oudompheng <remy@archlinux.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a get of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

[CCode (cprefix = "alpm_", cheader_filename="alpm.h")]
namespace Alpm {

	[SimpleType]
	[CCode (cname = "alpm_time_t", has_type_id = false)]
	public struct Time : uint64 {}

	/**
	* Library
	*/
	public unowned string version();

	[CCode (cname = "alpm_caps", cprefix = "ALPM_CAPABILITY_")]
	public enum Capabilities {
		NLS = (1 << 0),
		DOWNLOADER = (1 << 1),
		SIGNATURES = (1 << 2)
	}
	public Capabilities capabilities();

	public unowned Package? find_satisfier(Alpm.List<Package> pkgs, string depstring);

	public unowned Package? pkg_find(Alpm.List<Package> haystack, string needle);

	public int pkg_vercmp(string a, string b);

	public unowned Alpm.List<unowned Package?> find_group_pkgs(Alpm.List<DB> dbs, string name);

	/** Returns the string corresponding to an error number. */
	public unowned string strerror(Errno err);

	/**
	 * Handle
	 */
	[CCode (cname = "alpm_handle_t", free_function = "alpm_release")]
	[Compact]
	public class Handle {
		[CCode (cname = "alpm_initialize")]
		public static Handle? @new(string root, string dbpath, out Alpm.Errno error);

		public unowned string root {
			[CCode (cname = "alpm_option_get_root")] get;
		}
		public unowned string dbpath {
			[CCode (cname = "alpm_option_get_dbpath")] get;
		}
		public unowned string arch {
			[CCode (cname = "alpm_option_get_arch")] get;
			[CCode (cname = "alpm_option_set_arch")] set;
		}
		public unowned Alpm.List<unowned string?> cachedirs {
			[CCode (cname = "alpm_option_get_cachedirs")] get;
			[CCode (cname = "alpm_option_set_cachedirs")] set;
		}
		[CCode (cname = "alpm_option_add_cachedir")]
		public int add_cachedir(string cachedir);
		[CCode (cname = "alpm_option_remove_cachedir")]
		public int remove_cachedir(string cachedir);

		public unowned Alpm.List<unowned string?> hookdirs {
			[CCode (cname = "alpm_option_get_hookdirs")] get;
			[CCode (cname = "alpm_option_set_hookdirs")] set;
		}
		[CCode (cname = "alpm_option_add_hookdir")]
		public int add_hookdir(string hookdir);
		[CCode (cname = "alpm_option_remove_hookdir")]
		public int remove_hookdir(string hookdir);

		public unowned string logfile {
			[CCode (cname = "alpm_option_get_logfile")] get;
			[CCode (cname = "alpm_option_set_logfile")] set;
		}
		public unowned string lockfile {
			[CCode (cname = "alpm_option_get_lockfile")] get;
		}
		public unowned string gpgdir {
			[CCode (cname = "alpm_option_get_gpgdir")] get;
			[CCode (cname = "alpm_option_set_gpgdir")] set;
		}
		public int usesyslog {
			[CCode (cname = "alpm_option_get_usesyslog")] get;
			/** Sets whether to use syslog (0 is FALSE, TRUE otherwise). */
			[CCode (cname = "alpm_option_set_usesyslog")] set;
		}
		public unowned Alpm.List<unowned string?> noupgrades {
			[CCode (cname = "alpm_option_get_noupgrades")] get;
			[CCode (cname = "alpm_option_set_noupgrades")] set;
		}
		[CCode (cname = "alpm_option_add_noupgrade")]
		public int add_noupgrade(string path);
		[CCode (cname = "alpm_option_remove_noupgrade")]
		public int remove_noupgrade(string path);

		public unowned Alpm.List<unowned string?> noextracts {
			[CCode (cname = "alpm_option_get_noextracts")] get;
			[CCode (cname = "alpm_option_set_noextracts")] set;
		}
		[CCode (cname = "alpm_option_add_noextract")]
		public int add_noextract(string path);
		[CCode (cname = "alpm_option_remove_noextract")]
		public int remove_noextract(string path);
		[CCode (cname = "alpm_option_match_noextract")]
		public int match_noextract(string path);

		public unowned Alpm.List<unowned string?> ignorepkgs {
			[CCode (cname = "alpm_option_get_ignorepkgs")] get;
			[CCode (cname = "alpm_option_set_ignorepkgs")] set;
		}
		[CCode (cname = "alpm_option_add_ignorepkg")]
		public int add_ignorepkg(string pkg);
		[CCode (cname = "alpm_option_remove_ignorepkg")]
		public int remove_ignorepkg(string pkg);

		public unowned Alpm.List<unowned string?> ignoregroups {
			[CCode (cname = "alpm_option_get_ignoregroups")] get;
			[CCode (cname = "alpm_option_set_ignoregroups")] set;
		}
		[CCode (cname = "alpm_option_add_ignoregroup")]
		public int add_ignoregroup(string grp);
		[CCode (cname = "alpm_option_remove_ignorepkg")]
		public int remove_ignoregroup(string grp);

		public unowned Alpm.List<unowned Depend?> assumeinstalled {
			[CCode (cname = "alpm_option_get_assumeinstalled")] get;
			[CCode (cname = "alpm_option_set_assumeinstalled")] set;
		}
		[CCode (cname = "alpm_option_add_assumeinstalled")]
		public int add_assumeinstalled(Depend dep);
		[CCode (cname = "alpm_option_remove_assumeinstalled")]
		public int remove_assumeinstalled(Depend dep);

		public double deltaratio {
			[CCode (cname = "alpm_option_get_deltaratio")] get;
			[CCode (cname = "alpm_option_set_deltaratio")] set;
		}

		public int checkspace {
			[CCode (cname = "alpm_option_get_checkspace")] get;
			[CCode (cname = "alpm_option_set_checkspace")] set;
		}

		public unowned string dbext {
			[CCode (cname = "alpm_option_get_dbext")] get;
			[CCode (cname = "alpm_option_set_dbext")] set;
		}

		public Signature.Level defaultsiglevel {
			[CCode (cname = "alpm_option_get_default_siglevel")] get;
			[CCode (cname = "alpm_option_set_default_siglevel")] set;
		}
		public Signature.Level localfilesiglevel {
			[CCode (cname = "alpm_option_get_local_file_siglevel")] get;
			[CCode (cname = "alpm_option_set_local_file_siglevel")] set;
		}
		public Signature.Level remotefilesiglevel {
			[CCode (cname = "alpm_option_get_remote_file_siglevel")] get;
			[CCode (cname = "alpm_option_set_remote_file_siglevel")] set;
		}

		public unowned DB? localdb {
				[CCode (cname = "alpm_get_localdb")] get;
		}
		public unowned Alpm.List<unowned DB?> syncdbs {
				[CCode (cname = "alpm_get_syncdbs")] get;
		}

		public LogCallBack logcb {
			[CCode (cname = "alpm_option_get_logcb")] get;
			[CCode (cname = "alpm_option_set_logcb")] set;
		}
		public DownloadCallBack dlcb {
			[CCode (cname = "alpm_option_get_dlcb")] get;
			[CCode (cname = "alpm_option_set_dlcb")] set;
		}
		public FetchCallBack fetchcb {
			[CCode (cname = "alpm_option_get_fetchcb")] get;
			[CCode (cname = "alpm_option_set_fetchcb")] set;
		}
		public TotalDownloadCallBack totaldlcb {
			[CCode (cname = "alpm_option_get_totaldlcb")] get;
			[CCode (cname = "alpm_option_set_totaldlcb")] set;
		}
		public EventCallBack eventcb {
			[CCode (cname = "alpm_option_get_eventcb")] get;
			[CCode (cname = "alpm_option_set_eventcb")] set;
		}
		public QuestionCallBack questioncb {
			[CCode (cname = "alpm_option_get_questioncb")] get;
			[CCode (cname = "alpm_option_set_questioncb")] set;
		}
		public ProgressCallBack progresscb {
			[CCode (cname = "alpm_option_get_progresscb")] get;
			[CCode (cname = "alpm_option_set_progresscb")] set;
		}

		[CCode (cname = "alpm_unlock")]
		public int unlock();

		[CCode (cname = "alpm_register_syncdb")]
		public unowned DB? register_syncdb(string treename, Signature.Level level);
		[CCode (cname = "alpm_unregister_all_syncdbs")]
		public int unregister_all_syncdbs();

		// the return package can be freed except if it is added to a transaction,
		// it will be freed upon Handle.trans_release() invocation.
		[CCode (cname = "alpm_pkg_load")]
		public int load_tarball(string filename, int full, Signature.Level level, out Package pkg);

		/** Test if a package should be ignored.
		 * Checks if the package is ignored via IgnorePkg, or if the package is
		 * in a group ignored via IgnoreGroup.
		 * @param pkg the package to test
		 * @return 1 if the package should be ignored, 0 otherwise
		 */
		[CCode (cname = "alpm_pkg_should_ignore")]
		public int should_ignore(Package pkg);

		[CCode (cname = "alpm_fetch_pkgurl")]
		public string? fetch_pkgurl(string url);

		[CCode (cname = "alpm_find_dbs_satisfier")]
		public unowned Package? find_dbs_satisfier(Alpm.List<DB> dbs, string depstring);

		/** Returns the current error code from the handle. */
		[CCode (cname = "alpm_errno")]
		public Errno errno();

		/** Returns the bitfield of flags for the current transaction.*/
		[CCode (cname = "alpm_trans_get_flags")]
		public TransFlag trans_get_flags();

		/** Returns a list of packages added by the transaction.*/
		[CCode (cname = "alpm_trans_get_add")]
		public unowned Alpm.List<unowned Package?> trans_to_add();

		/** Returns the list of packages removed by the transaction.*/
		[CCode (cname = "alpm_trans_get_remove")]
		public unowned Alpm.List<unowned Package?> trans_to_remove();

		/** Initialize the transaction.
		* @param flags flags of the transaction (like nodeps, etc)
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_trans_init")]
		public int trans_init(TransFlag transflags);

		/** Prepare a transaction.
		* @param an alpm_list where detailed description of an error
		* can be dumped (i.e. list of conflicting packages)
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_trans_prepare")]
		public int trans_prepare(out Alpm.List<void*> data);

		/** Commit a transaction.
		* @param an alpm_list where detailed description of an error
		* can be dumped (i.e. list of conflicting files)
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_trans_commit")]
		public int trans_commit(out Alpm.List<void*> data);

		/** Interrupt a transaction.
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_trans_interrupt")]
		public int trans_interrupt();
		
		/** Release a transaction.
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_trans_release")]
		public int trans_release();

		/** Search for packages to upgrade and add them to the transaction.
		* @param enable_downgrade allow downgrading of packages if the remote version is lower
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_sync_sysupgrade")]
		public int trans_sysupgrade(int enable_downgrade);

		/** Add a package to the transaction.
		* If the package was loaded by load_file(), it will be freed upon
		* trans_release() invocation.
		* @param pkg the package to add
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_add_pkg")]
		public int trans_add_pkg(Package pkg);

		/** Add a package removal action to the transaction.
		* @param pkg the package to uninstall
		* @return 0 on success, -1 on error (Errno is set accordingly)
		*/
		[CCode (cname = "alpm_remove_pkg")]
		public int trans_remove_pkg(Package pkg);
	}

	/**
	 * Databases
	 */
	[CCode (cname = "alpm_db_t", cprefix = "alpm_db_")]
	[Compact]
	public class DB {
		public static int unregister(owned DB db);

		public unowned string name {
			[CCode (cname = "alpm_db_get_name")] get;
		}

		public Signature.Level siglevel {
			[CCode (cname = "alpm_db_get_siglevel")] get;
		}

		public unowned Alpm.List<unowned string?> servers {
			[CCode (cname = "alpm_db_get_servers")] get;
			[CCode (cname = "alpm_db_set_servers")] set;
		}

		public unowned Alpm.List<unowned Package?> pkgcache {
			[CCode (cname = "alpm_db_get_pkgcache")] get;
		}

		public unowned Alpm.List<unowned Group?> groupcache {
			[CCode (cname = "alpm_db_get_groupcache")] get;
		}

		public Usage usage {
			[CCode (cname = "alpm_db_get_usage")] get;
			[CCode (cname = "alpm_db_set_usage")] set;
		}

		[CCode (cname = "alpm_db_usage_t", cprefix = "ALPM_DB_USAGE_")]
		public enum Usage {
			SYNC = 1,
			SEARCH = (1 << 1),
			INSTALL = (1 << 2),
			UPGRADE = (1 << 3),
			ALL = (1 << 4) - 1,
		}

		public int add_server(string url);
		public int remove_server(string url);

		[CCode (instance_pos = 1.1)]
		public int update(int force);

		public unowned Package? get_pkg(string name);
		public unowned Group? get_group(string name);
		public Alpm.List<unowned Package?> search(Alpm.List<string> needles);

		public int check_pgp_signature(out SigList siglist);
	}

	/**
	 * Packages
	 */
	[CCode (cname = "alpm_pkg_t", cprefix = "alpm_pkg_", free_function = "alpm_pkg_free")]
	[Compact]
	public class Package {
		/* properties */
		public unowned string filename {
			[CCode (cname = "alpm_pkg_get_filename")] get;
		}
		public unowned string name {
			[CCode (cname = "alpm_pkg_get_name")] get;
		}
		public unowned string version {
			[CCode (cname = "alpm_pkg_get_version")] get;
		}
		public unowned string pkgbase {
			[CCode (cname = "alpm_pkg_get_base")] get;
		}
		public From origin {
			[CCode (cname = "alpm_pkg_get_origin")] get;
		}
		public unowned string desc {
			[CCode (cname = "alpm_pkg_get_desc")] get;
		}
		public unowned string url {
			[CCode (cname = "alpm_pkg_get_url")] get;
		}
		public Time builddate {
			[CCode (cname = "alpm_pkg_get_builddate")] get;
		}
		public Time installdate {
			[CCode (cname = "alpm_pkg_get_installdate")] get;
		}
		public unowned string packager {
			[CCode (cname = "alpm_pkg_get_packager")] get;
		}
		public unowned string md5sum {
			[CCode (cname = "alpm_pkg_get_md5sum")] get;
		}
		public unowned string sha256sum {
			[CCode (cname = "alpm_pkg_get_sha256sum")] get;
		}
		public unowned string arch {
			[CCode (cname = "alpm_pkg_get_arch")] get;
		}

		/** Returns the size of the package. This is only available for sync database
		 * packages and package files, not those loaded from the local database.
		 */
		public uint64 size {
			[CCode (cname = "alpm_pkg_get_size")] get;
		}

		public uint64 isize {
			[CCode (cname = "alpm_pkg_get_isize")] get;
		}
		public uint64 download_size {
			[CCode (cname = "alpm_pkg_download_size")] get;
		}
		public Reason reason {
			[CCode (cname = "alpm_pkg_get_reason")] get;
			/** This must be a Package from the local database
			 * or this method will fail (Errno is set accordingly).
			 */
			[CCode (cname = "alpm_pkg_set_reason")] set;
		}
		public unowned Alpm.List<unowned string?> licenses {
			[CCode (cname = "alpm_pkg_get_licenses")] get;
		}
		public unowned Alpm.List<unowned string?> groups {
			[CCode (cname = "alpm_pkg_get_groups")] get;
		}
		public unowned Alpm.List<unowned Depend?> depends {
			[CCode (cname = "alpm_pkg_get_depends")] get;
		}
		public unowned Alpm.List<unowned Depend?> optdepends {
			[CCode (cname = "alpm_pkg_get_optdepends")] get;
		}
		public unowned Alpm.List<unowned Depend?> conflicts {
			[CCode (cname = "alpm_pkg_get_conflicts")] get;
		}
		public unowned Alpm.List<unowned Depend?> provides {
			[CCode (cname = "alpm_pkg_get_provides")] get;
		}
		public unowned Alpm.List<unowned string?> deltas {
			[CCode (cname = "alpm_pkg_get_deltas")] get;
		}
		public unowned Alpm.List<unowned Depend?> replaces {
			[CCode (cname = "alpm_pkg_get_replaces")] get;
		}
		public unowned FileList files {
			[CCode (cname = "alpm_pkg_get_files")] get;
		}
		public unowned Alpm.List<unowned string?> unused_deltas {
			[CCode (cname = "alpm_pkg_unused_deltas")] get;
		}
		public unowned Alpm.List<unowned Backup?> backups {
			[CCode (cname = "alpm_pkg_get_backup")] get;
		}
		public unowned DB? db {
			[CCode (cname = "alpm_pkg_get_db")] get;
		}
		public unowned string base64_sig {
			[CCode (cname = "alpm_pkg_get_base64_sig")] get;
		}
		public Validation validation {
			[CCode (cname = "alpm_pkg_get_validation")] get;
		}
		// TODO: changelog functions

		/** Package install reasons. */
		[CCode (cname = "alpm_pkgreason_t", cprefix = "ALPM_PKG_REASON_")]
		public enum Reason {
			/** Explicitly requested by the user. */
			EXPLICIT = 0,
			/** Installed as a dependency for another package. */
			DEPEND = 1
		}

		/** Location a package object was loaded from. */
		[CCode (cname = "alpm_pkgfrom_t", cprefix = "ALPM_PKG_FROM_")]
		public enum From {
			FILE = 1,
			LOCALDB,
			SYNCDB
		}

		/** Method used to validate a package. */
		[CCode (cname = "alpm_pkgvalidation_t", cprefix = "ALPM_PKG_VALIDATION_")]
		public enum Validation {
			UNKNOWN = 0,
			NONE = (1 << 0),
			MD5SUM = (1 << 1),
			SHA256SUM = (1 << 2),
			SIGNATURE = (1 << 3)
		}

		[CCode (cname = "alpm_package_operation_t", cprefix = "ALPM_PACKAGE_")]
		public enum Operation {
			/** Package (to be) installed. (No oldpkg) */
			INSTALL = 1,
			/** Package (to be) upgraded */
			UPGRADE,
			/** Package (to be) re-installed. */
			REINSTALL,
			/** Package (to be) downgraded. */
			DOWNGRADE,
			/** Package (to be) removed. (No newpkg) */
			REMOVE
		}

		public int checkmd5sum();
		public int has_scriptlet();

		public Alpm.List<string?> compute_requiredby();
		public Alpm.List<string?> compute_optionalfor();

		[CCode (cname = "alpm_sync_newversion")]
		public unowned Package? sync_newversion(Alpm.List<DB> dbs);

		public int check_pgp_signature(out SigList siglist);
	}

	/** Dependency */
	[CCode (cname = "alpm_depend_t", free_function = "alpm_dep_free")]
	[Compact]
	public class Depend {
		public string name;
		public string version;
		public string desc;
		public ulong name_hash;
		public Mode mod;

		public static Depend from_string(string depstring);

		[CCode (cname = "alpm_dep_compute_string")]
		public string compute_string();

		/** Types of version constraints in dependency specs. */
		[CCode (cname = "alpm_depmod_t", cprefix = "ALPM_DEP_MOD_")]
		public enum Mode {
			/** No version constraint */
			ANY = 1,
			/** Test version equality (package=x.y.z) */
			EQ,
			/** Test for at least a version (package>=x.y.z) */
			GE,
			/** Test for at most a version (package<=x.y.z) */
			LE,
			/** Test for greater than some version (package>x.y.z) */
			GT,
			/** Test for less than some version (package<x.y.z) */
			LT
		}
	}

	/** Missing dependency */
	[CCode (cname = "alpm_depmissing_t", free_function = "alpm_depmissing_free")]
	[Compact]
	public class DepMissing {
		public string target;
		public unowned Depend depend;
		// this is used only in the case of a remove dependency error
		public string causingpkg;
	}

	/** Conflict */
	[CCode (cname = "alpm_conflict_t", free_function = "alpm_conflict_free")]
	[Compact]
	public class Conflict {
		public ulong package1_hash;
		public ulong package2_hash;
		public string package1;
		public string package2;
		public unowned Depend reason;
	}

	/** File conflict */
	[CCode (cname = "alpm_fileconflict_t", free_function = "alpm_fileconflict_free")]
	[Compact]
	public class FileConflict {
		public string target;
		public Type type;
		public string file;
		public string ctarget;
		/**
		* File conflict type.
		* Whether the conflict results from a file existing on the filesystem, or with
		* another target in the transaction.
		*/
		[CCode (cname = "alpm_fileconflicttype_t", cprefix = "ALPM_FILECONFLICT_")]
		public enum Type {
			TARGET = 1,
			FILESYSTEM
		}
	}

	/** Package group */
	[CCode (cname = "alpm_group_t", has_type_id = false)]
	[Compact]
	public class Group {
		public string name;
		public unowned Alpm.List<unowned Package?> packages;
	}

	/** Package upgrade delta */
	[CCode (cname = "alpm_delta_t", has_type_id = false)]
	[Compact]
	public class Delta {
		/** filename of the delta patch */
		public string delta;
		/** md5sum of the delta file */
		public string delta_md5;
		/** filename of the 'before' file */
		public string from;
		/** filename of the 'after' file */
		public string to;
		/** filesize of the delta file */
		public uint64 delta_size;
		/** download filesize of the delta file */
		public uint64 download_size;
	}

	/** File in a package */
	[CCode (cname = "alpm_file_t", has_type_id = false)]
	[Compact]
	public class File {
		public string name;
		public uint64 size;
		public uint64 mode;
	}

	/** Package filelist container */
	[CCode (cname = "alpm_filelist_t", has_type_id = false)]
	[Compact]
	public class FileList {
		public size_t count;
		[CCode (array_length_cname = "count", array_length_type = "size_t")]
		public unowned Alpm.File[] files;
		/** Determines whether a package filelist contains a given path.
		 * The provided path should be relative to the install root with no leading
		 * slashes, e.g. "etc/localtime". When searching for directories, the path must
		 * have a trailing slash.
		 * @param path the path to search for in the package
		 * @return a pointer to the matching file or NULL if not found
		 */
		[CCode (cname = "alpm_filelist_contains")]
		unowned Alpm.File? contains(string path);
	}

	/** Local package or package file backup entry */
	[CCode (cname = "alpm_backup_t", has_type_id = false)]
	[Compact]
	public class Backup {
		public string name;
		public string hash;
	}

	namespace Signature {
		[CCode (cname = "alpm_pgpkey_t", has_type_id = false)]
		[Compact]
		public class PGPKey {
			public void *data;
			public string fingerprint;
			public string uid;
			public string name;
			public string email;
			public Time created;
			public Time expires;
			public uint length;
			public uint revoked;
			public string pubkey_algo;
		}

		/**
		* Signature result. Contains the key, status, and validity of a given
		* signature.
		*/
		[CCode (cname = "alpm_sigresult_t", has_type_id = false)]
		[Compact]
		public class Result {
			public PGPKey key;
			public Status status;
			public Validity validity;
		}

			/** PGP signature verification options */
		[CCode (cname = "alpm_siglevel_t", cprefix = "ALPM_SIG_")]
		public enum Level {
			PACKAGE = (1 << 0),
			PACKAGE_OPTIONAL = (1 << 1),
			PACKAGE_MARGINAL_OK = (1 << 2),
			PACKAGE_UNKNOWN_OK = (1 << 3),

			DATABASE = (1 << 10),
			DATABASE_OPTIONAL = (1 << 11),
			DATABASE_MARGINAL_OK = (1 << 12),
			DATABASE_UNKNOWN_OK = (1 << 13),

			USE_DEFAULT = (1 << 31)
		}

		/** PGP signature verification status return codes */
		[CCode (cname = "alpm_sigstatus_t", cprefix = "ALPM_SIGSTATUS_")]
		public enum Status {
			VALID,
			KEY_EXPIRED,
			SIG_EXPIRED,
			KEY_UNKNOWN,
			KEY_DISABLED,
			INVALID
		}

		/** PGP signature verification status return codes */
		[CCode (cname = "alpm_sigvalidity_t", cprefix = "ALPM_SIGVALIDITY_")]
		public enum Validity {
			FULL,
			MARGINAL,
			NEVER,
			UNKNOWN
		}
	}

	[CCode (cname = "alpm_siglist_t", free_function = "alpm_siglist_cleanup")]
	[Compact]
	public class SigList {
		public size_t count;
		[CCode (array_length_cname = "count", array_length_type = "size_t")]
		public unowned Signature.Result[] results;
	}

	/** Hooks */
	[CCode (cname = "alpm_hook_when_t", cprefix = "ALPM_HOOK_")]
	public enum HookWhen {
		PRE_TRANSACTION = 1,
		POST_TRANSACTION
	}

	/** Logging Levels */
	[CCode (cname = "alpm_loglevel_t", cprefix = "ALPM_LOG_")]
	public enum LogLevel {
		ERROR    = 1,
		WARNING  = (1 << 1),
		DEBUG    = (1 << 2),
		FUNCTION = (1 << 3)
	}

	/** Log callback */
	[CCode (cname = "alpm_cb_log", has_type_id = false, has_target = false)]
	public delegate void LogCallBack(LogLevel level, string fmt, va_list args);

	namespace Event {
		/**
		* Type of events.
		*/
		[CCode (cname = "alpm_event_type_t", cprefix = "ALPM_EVENT_")]
		public enum Type {
			/** Dependencies will be computed for a package. */
			CHECKDEPS_START = 1,
			/** Dependencies were computed for a package. */
			CHECKDEPS_DONE,
			/** File conflicts will be computed for a package. */
			FILECONFLICTS_START,
			/** File conflicts were computed for a package. */
			FILECONFLICTS_DONE,
			/** Dependencies will be resolved for target package. */
			RESOLVEDEPS_START,
			/** Dependencies were resolved for target package. */
			RESOLVEDEPS_DONE,
			/** Inter-conflicts will be checked for target package. */
			INTERCONFLICTS_START,
			/** Inter-conflicts were checked for target package. */
			INTERCONFLICTS_DONE,
			/** Processing the package transaction is starting. */
			TRANSACTION_START,
			/** Processing the package transaction is finished. */
			TRANSACTION_DONE,
			/** Package will be installed/upgraded/downgraded/re-installed/removed; See
			 * PackageOperation for arguments. */
			PACKAGE_OPERATION_START,
			/** Package was installed/upgraded/downgraded/re-installed/removed; See
			 * PackageOperation for arguments. */
			PACKAGE_OPERATION_DONE,
			/** Target package's integrity will be checked. */
			INTEGRITY_START,
			/** Target package's integrity was checked. */
			INTEGRITY_DONE,
			/** Target package will be loaded. */
			LOAD_START,
			/** Target package is finished loading. */
			LOAD_DONE,
			/** Target delta's integrity will be checked. */
			DELTA_INTEGRITY_START,
			/** Target delta's integrity was checked. */
			DELTA_INTEGRITY_DONE,
			/** Deltas will be applied to packages. */
			DELTA_PATCHES_START,
			/** Deltas were applied to packages. */
			DELTA_PATCHES_DONE,
			/** Delta patch will be applied to target package; See
			* DeltaPatch for arguments. */
			DELTA_PATCH_START,
			/** Delta patch was applied to target package. */
			DELTA_PATCH_DONE,
			/** Delta patch failed to apply to target package. */
			DELTA_PATCH_FAILED,
			/** Scriptlet has printed information; See ScriptletInfo for
			* arguments. */
			SCRIPTLET_INFO,
			/** Files will be downloaded from a repository. */
			RETRIEVE_START,
			/** Files were downloaded from a repository. */
			RETRIEVE_DONE,
			/** Not all files were successfully downloaded from a repository. */
			RETRIEVE_FAILED,
			/** A file will be downloaded from a repository; See PkgDownload
			 * for arguments */
			PKGDOWNLOAD_START,
			/** A file was downloaded from a repository; See PkgDownload
			 * for arguments */
			PKGDOWNLOAD_DONE,
			/** A file failed to be downloaded from a repository; See
			 * PkgDownload for arguments */
			PKGDOWNLOAD_FAILED,
			/** Disk space usage will be computed for a package. */
			DISKSPACE_START,
			/** Disk space usage was computed for a package. */
			DISKSPACE_DONE,
			/** An optdepend for another package is being removed; See
			 * OptDepRemoval for arguments. */
			OPTDEP_REMOVAL,
			/** A configured repository database is missing; See
			 * DatabaseMissing for arguments. */
			DATABASE_MISSING,
			/** Checking keys used to create signatures are in keyring. */
			KEYRING_START,
			/** Keyring checking is finished. */
			KEYRING_DONE,
			/** Downloading missing keys into keyring. */
			KEY_DOWNLOAD_START,
			/** Key downloading is finished. */
			KEY_DOWNLOAD_DONE,
			/** A .pacnew file was created; See PacnewCreated for arguments. */
			PACNEW_CREATED,
			/** A .pacsave file was created; See PacsaveCreated for
			 * arguments */
			PACSAVE_CREATED,
			/** Processing hooks will be started. */
			HOOK_START,
			/** Processing hooks is finished. */
			HOOK_DONE,
			/** A hook is starting */
			HOOK_RUN_START,
			/** A hook has finnished runnning */
			HOOK_RUN_DONE
		}

		[CCode (cname = "alpm_event_any_t", has_type_id = false)]
		[Compact]
		public class Any {
			/** Type of event. */
			public Type type;
		}

		[CCode (cname = "alpm_event_package_operation_t", has_type_id = false)]
		[Compact]
		public class PackageOperation {
			/** Type of event. */
			public Type type;
			/** Type of operation. */
			public Package.Operation operation;
			/** Old package. */
			public unowned Package oldpkg;
			/** New package. */
			public unowned Package newpkg;
		}

		[CCode (cname = "alpm_event_optdep_removal_t", has_type_id = false)]
		[Compact]
		public class OptDepRemoval {
			/** Type of event. */
			public Type type;
			/** Package with the optdep. */
			public unowned Package pkg;
			/** Optdep being removed. */
			public unowned Depend optdep;
		}

		[CCode (cname = "alpm_event_delta_patch_t", has_type_id = false)]
		[Compact]
		public class DeltaPatch {
			/** Type of event. */
			public Type type;
			/** Delta info */
			public Delta delta;
		}

		[CCode (cname = "alpm_event_scriptlet_info_t", has_type_id = false)]
		[Compact]
		public class ScriptletInfo {
			/** Type of event. */
			public Type type;
			/** Line of scriptlet output. */
			public unowned string line;
		}

		[CCode (cname = "alpm_event_database_missing_t", has_type_id = false)]
		[Compact]
		public class DatabaseMissing {
			/** Type of event. */
			public Type type;
			/** Name of the database. */
			public unowned string dbname;
		}

		[CCode (cname = "alpm_event_pkgdownload_t", has_type_id = false)]
		[Compact]
		public class PkgDownload {
			/** Type of event. */
			public Type type;
			/** Name of the file */
			public unowned string file;
		}

		[CCode (cname = "alpm_event_pacnew_created_t", has_type_id = false)]
		[Compact]
		public class PacnewCreated {
			/** Type of event. */
			public Type type;
			/** Whether the creation was result of a NoUpgrade or not */
			public int from_noupgrade;
			/** Old package. */
			public unowned Package oldpkg;
			/** New Package. */
			public unowned Package newpkg;
			/** Filename of the file without the .pacnew suffix */
			public unowned string file;
		}

		[CCode (cname = "alpm_event_pacsave_created_t", has_type_id = false)]
		[Compact]
		public class PacsaveCreated {
			/** Type of event. */
			public Type type;
			/** Old package. */
			public unowned Package oldpkg;
			/** Filename of the file without the .pacsave suffix. */
			public unowned string file;
		}

		[CCode (cname = "alpm_event_hook_t", has_type_id = false)]
		[Compact]
		public class Hook {
			/** Type of event.*/
			public Type type;
			/** Type of hooks. */
			public HookWhen when;
		}

		[CCode (cname = "alpm_event_hook_run_t", has_type_id = false)]
		[Compact]
		public class HookRun {
			/** Type of event.*/
			public Type type;
			/** Name of hook */
			public unowned string name;
			/** Description of hook to be outputted */
			public unowned string desc;
			/** position of hook being run */
			public size_t position;
			/** total hooks being run */
			public size_t total;
		}

		/** This is an union passed to the callback, that allows the frontend to know
		 * which type of event was triggered (via type). It is then possible to
		 * typecast the pointer to the right structure, or use the union field, in order
		 * to access event-specific data. */
		[CCode (cname = "alpm_event_t", has_type_id = false)]
		[Compact]
		public class Data {
			/** Type of event. */
			public Type type;
			// PackageOperation package_operation;
			[CCode (cname = "package_operation.operation")]
			public Package.Operation package_operation_operation;
			[CCode (cname = "package_operation.oldpkg")]
			public unowned Package package_operation_oldpkg;
			[CCode (cname = "package_operation.newpkg")]
			public unowned Package package_operation_newpkg;
			// OptDepRemoval optdep_removal;
			[CCode (cname = "optdep_removal.pkg")]
			public unowned Package optdep_removal_pkg;
			[CCode (cname = "optdep_removal.optdep")]
			public unowned Depend optdep_removal_optdep;
			// DeltaPatch delta_patch;
			[CCode (cname = "delta_patch.delta")]
			public Delta delta_patch_delta;
			// ScriptletInfo scriptlet_info;
			[CCode (cname = "scriptlet_info.line")]
			public unowned string scriptlet_info_line;
			// DatabaseMissing database_missing;
			[CCode (cname = "database_missing.dbname")]
			public unowned string database_missing_dbname;
			// PkgDownload pkgdownload;
			[CCode (cname = "pkgdownload.file")]
			public unowned string pkgdownload_file;
			// PacnewCreated pacnew_created;
			[CCode (cname = "pacnew_created.from_noupgrade")]
			public int pacnew_created_from_noupgrade;
			[CCode (cname = "pacnew_created.oldpkg")]
			public unowned Package pacnew_created_oldpkg;
			[CCode (cname = "pacnew_created.newpkg")]
			public unowned Package pacnew_created_newpkg;
			[CCode (cname = "pacnew_created.file")]
			public unowned string pacnew_created_file;
			// PacsaveCreated pacsave_created;
			[CCode (cname = "pacsave_created.oldpkg")]
			public unowned Package pacsave_created_oldpkg;
			[CCode (cname = "pacsave_created.file")]
			public unowned string pacsave_created_file;
			// Hook hook;
			[CCode (cname = "hook.when")]
			public HookWhen hook_when;
			// HookRun hook_run;
			[CCode (cname = "hook_run.name")]
			public unowned string hook_run_name;
			[CCode (cname = "hook_run.desc")]
			public unowned string hook_run_desc;
			[CCode (cname = "hook_run.position")]
			public size_t hook_run_position;
			[CCode (cname = "hook_run.total")]
			public size_t hook_run_total;
		}
	}

	/** Event callback */
	[CCode (cname = "alpm_cb_event", has_type_id = false, has_target = false)]
	public delegate void EventCallBack (Event.Data data);

	namespace Question {
		/**
		* Type of questions.
		* Unlike the events or progress enumerations, this enum has bitmask values
		* so a frontend can use a bitmask map to supply preselected answers to the
		* different types of questions.
		*/
		[CCode (cname = "alpm_question_type_t", cprefix = "ALPM_QUESTION_")]
		public enum Type {
			INSTALL_IGNOREPKG = (1 << 0),
			REPLACE_PKG = (1 << 1),
			CONFLICT_PKG = (1 << 2),
			CORRUPTED_PKG = (1 << 3),
			REMOVE_PKGS = (1 << 4),
			SELECT_PROVIDER = (1 << 5),
			IMPORT_KEY = (1 << 6)
		}

		[CCode (cname = "alpm_question_any_t", has_type_id = false)]
		[Compact]
		public class Any {
			/** Type of question. */
			public Type type;
			/** Answer. */
			public int answer;
		}

		[CCode (cname = "alpm_question_install_ignorepkg_t", has_type_id = false)]
		[Compact]
		public class InstallIgnorePkg {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to install pkg anyway. */
			public int install;
			// Package in IgnorePkg/IgnoreGroup.
			public unowned Package pkg;
		}

		[CCode (cname = "alpm_question_replace_t", has_type_id = false)]
		[Compact]
		public class Replace {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to replace oldpkg with newpkg. */
			public int replace;
			// Package to be replaced.
			public unowned Package oldpkg;
			// Package to replace with. */
			public unowned Package newpkg;
			// DB of newpkg
			public unowned DB newdb;
		}

		[CCode (cname = "alpm_question_conflict_t", has_type_id = false)]
		[Compact]
		public class Conflict {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to remove conflict->package2. */
			public int remove;
			/** Conflict info. */
			public Alpm.Conflict conflict;
		}

		[CCode (cname = "alpm_question_corrupted_t", has_type_id = false)]
		[Compact]
		public class Corrupted {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to remove filepath. */
			public int remove;
			/** Filename to remove */
			public unowned string filepath;
			/** Error code indicating the reason for package invalidity */
			public Errno reason;
		}

		[CCode (cname = "alpm_question_remove_pkgs_t", has_type_id = false)]
		[Compact]
		public class RemovePkgs {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to skip packages. */
			public int skip;
			/** List of alpm_pkg_t* with unresolved dependencies. */
			public unowned Alpm.List<unowned Package?> packages;
		}

		[CCode (cname = "alpm_question_select_provider_t", has_type_id = false)]
		[Compact]
		public class SelectProvider {
			/** Type of question. */
			public Type type;
			/** Answer: which provider to use (index from providers). */
			public int use_index;
			/** List of alpm_pkg_t* as possible providers. */
			public unowned Alpm.List<unowned Package?> providers;
			/** What providers provide for. */
			public unowned Depend depend;
		}

		[CCode (cname = "alpm_question_import_key_t", has_type_id = false)]
		[Compact]
		public class ImportKey {
			/** Type of question. */
			public Type type;
			/** Answer: whether or not to import key. */
			public int import;
			/** The key to import. */
			public Signature.PGPKey key;
		}

		/** This is an union passed to the callback, that allows the frontend to know
		 * which type of question was triggered (via type). It is then possible to
		 * typecast the pointer to the right structure, or use the union field, in order
		 * to access question-specific data. */
		[CCode (cname = "alpm_question_t", has_type_id = false)]
		[Compact]
		public class Data {
			public Type type;
			// Any any;
			[CCode (cname = "any.answer")]
			public int any_answer;
			// InstallIgnorePkg install_ignorepkg;
			[CCode (cname = "install_ignorepkg.install")]
			public int install_ignorepkg_install;
			[CCode (cname = "install_ignorepkg.pkg")]
			public unowned Package install_ignorepkg_pkg;
			// Replace replace;
			[CCode (cname = "replace.replace")]
			public int replace_replace;
			[CCode (cname = "replace.oldpkg")]
			public unowned Package replace_oldpkg;
			[CCode (cname = "replace.newpkg")]
			public unowned Package replace_newpkg;
			[CCode (cname = "replace.newdb")]
			public unowned DB replace_newdb;
			// Conflict conflict;
			[CCode (cname = "conflict.remove")]
			public int conflict_remove;
			[CCode (cname = "conflict.conflict")]
			public Alpm.Conflict conflict_conflict;
			// Corrupted corrupted;
			[CCode (cname = "corrupted.remove")]
			public int corrupted_remove;
			[CCode (cname = "corrupted.filepath")]
			public unowned string corrupted_filepath;
			[CCode (cname = "corrupted.reason")]
			public Errno corrupted_reason;
			// RemovePkgs remove_pkgs;
			[CCode (cname = "remove_pkgs.skip")]
			public int remove_pkgs_skip;
			[CCode (cname = "remove_pkgs.packages")]
			public unowned Alpm.List<unowned Package?> remove_pkgs_packages;
			// SelectProvider select_provider;
			[CCode (cname = "select_provider.use_index")]
			public int select_provider_use_index;
			[CCode (cname = "select_provider.providers")]
			public unowned Alpm.List<unowned Package?> select_provider_providers;
			[CCode (cname = "select_provider.depend")]
			public unowned Depend select_provider_depend;
			// ImportKey import_key;
			[CCode (cname = "import_key.import")]
			public int import_key_import;
			[CCode (cname = "import_key.key")]
			public Signature.PGPKey import_key_key;
		}
	}

	/** Question callback */
	[CCode (cname = "alpm_cb_question", has_type_id = false, has_target = false)]
	public delegate void QuestionCallBack (Question.Data data);

	/** Progress */
	[CCode (cname = "alpm_progress_t", cprefix = "ALPM_PROGRESS_")]
	public enum Progress {
		ADD_START,
		UPGRADE_START,
		DOWNGRADE_START,
		REINSTALL_START,
		REMOVE_START,
		CONFLICTS_START,
		DISKSPACE_START,
		INTEGRITY_START,
		LOAD_START,
		KEYRING_START
	}

	/** Progress callback */
	[CCode (cname = "alpm_cb_progress", has_type_id = false, has_target = false)]
	public delegate void ProgressCallBack (Progress progress, string pkgname, int percent, uint n_targets, uint current_target);

	/** Type of download progress callbacks.
	* @param filename the name of the file being downloaded
	* @param xfered the number of transferred bytes
	* @param total the total number of bytes to transfer
	*/
	[CCode (cname = "alpm_cb_download", has_type_id = false, has_target = false)]
	public delegate void DownloadCallBack (string filename, uint64 xfered, uint64 total);

	[CCode (cname = "alpm_cb_totaldl", has_type_id = false, has_target = false)]
	public delegate void TotalDownloadCallBack (uint64 total);

	/** A callback for downloading files
	* @param url the URL of the file to be downloaded
	* @param localpath the directory to which the file should be downloaded
	* @param force whether to force an update, even if the file is the same
	* @return 0 on success, 1 if the file exists and is identical, -1 on
	* error.
	*/
	[CCode (cname = "alpm_cb_fetch", has_type_id = false, has_target = false)]
	public delegate int FetchCallBack (string url, string localpath, int force);

	/** Transaction flags */
	[CCode (cname = "alpm_transflag_t", cprefix = "ALPM_TRANS_FLAG_")]
	public enum TransFlag {
		/** Ignore dependency checks. */
		NODEPS = 1,
		/** Ignore file conflicts and overwrite files. */
		FORCE = (1 << 1),
		/** Delete files even if they are tagged as backup. */
		NOSAVE = (1 << 2),
		/** Ignore version numbers when checking dependencies. */
		NODEPVERSION = (1 << 3),
		/** Remove also any packages depending on a package being removed. */
		CASCADE = (1 << 4),
		/** Remove packages and their unneeded deps (not explicitly installed). */
		RECURSE = (1 << 5),
		/** Modify database but do not commit changes to the filesystem. */
		DBONLY = (1 << 6),
		/** Use ALPM_PKG_REASON_DEPEND when installing packages. */
		ALLDEPS = (1 << 8),
		/** Only download packages and do not actually install. */
		DOWNLOADONLY = (1 << 9),
		/** Do not execute install scriptlets after installing. */
		NOSCRIPTLET = (1 << 10),
		/** Ignore dependency conflicts. */
		NOCONFLICTS = (1 << 11),
		/** Do not install a package if it is already installed and up to date. */
		NEEDED = (1 << 13),
		/** Use ALPM_PKG_REASON_EXPLICIT when installing packages. */
		ALLEXPLICIT = (1 << 14),
		/** Do not remove a package if it is needed by another one. */
		UNNEEDED = (1 << 15),
		/** Remove also explicitly installed unneeded deps (use with ALPM_TRANS_FLAG_RECURSE). */
		RECURSEALL = (1 << 16),
		/** Do not lock the database during the operation. */
		NOLOCK = (1 << 17)
	}

	/**
	 * Errnos
	 */
	[CCode (cname = "alpm_errno_t", cprefix = "ALPM_ERR_")]
	public enum Errno {
		MEMORY = 1,
		SYSTEM,
		BADPERMS,
		NOT_A_FILE,
		NOT_A_DIR,
		WRONG_ARGS,
		DISK_SPACE,
		/* Interface */
		HANDLE_NULL,
		HANDLE_NOT_NULL,
		HANDLE_LOCK,
		/* Databases */
		DB_OPEN,
		DB_CREATE,
		DB_NULL,
		DB_NOT_NULL,
		DB_NOT_FOUND,
		DB_INVALID,
		DB_INVALID_SIG,
		DB_VERSION,
		DB_WRITE,
		DB_REMOVE,
		/* Servers */
		SERVER_BAD_URL,
		SERVER_NONE,
		/* Transactions */
		TRANS_NOT_NULL,
		TRANS_NULL,
		TRANS_DUP_TARGET,
		TRANS_NOT_INITIALIZED,
		TRANS_NOT_PREPARED,
		TRANS_ABORT,
		TRANS_TYPE,
		TRANS_NOT_LOCKED,
		TRANS_HOOK_FAILED,
		/* Packages */
		PKG_NOT_FOUND,
		PKG_IGNORED,
		PKG_INVALID,
		PKG_INVALID_CHECKSUM,
		PKG_INVALID_SIG,
		PKG_MISSING_SIG,
		PKG_OPEN,
		PKG_CANT_REMOVE,
		PKG_INVALID_NAME,
		PKG_INVALID_ARCH,
		PKG_REPO_NOT_FOUND,
		/* Signatures */
		SIG_MISSING,
		SIG_INVALID,
		/* Deltas */
		DLT_INVALID,
		DLT_PATCHFAILED,
		/* Dependencies */
		UNSATISFIED_DEPS,
		CONFLICTING_DEPS,
		FILE_CONFLICTS,
		/* Misc */
		RETRIEVE,
		INVALID_REGEX,
		/* External library errors */
		LIBARCHIVE,
		LIBCURL,
		EXTERNAL_DOWNLOAD,
		GPGME
	}

	[CCode (cname = "alpm_list_t", cprefix = "alpm_list_", cheader_filename = "alpm_list.h",
			dup_function = "alpm_list_copy", free_function = "alpm_list_free")]
	[Compact]
	public class List<G> {

		/* comparator */
		[CCode (cname = "alpm_list_fn_cmp", has_target = false)]
		public delegate int CompareFunc<G>(G a, G b);
		/* deallocator */
		[CCode (cname = "alpm_list_fn_free", has_target = false)]
		public delegate void FreeFunc<G>(G a);

		[CCode (cname = "alpm_list_count")]
		public size_t length();

		/* fields */
		public G data;
		public List<G> next;
		public unowned List<G> prev;

		/* item mutators */
		[ReturnsModifiedPointer ()]
		[CCode (cname = "alpm_list_add")]
		public void append(owned G data);

		[ReturnsModifiedPointer ()]
		public void join(owned List<G> list);

		[CCode (cname = "alpm_list_msort", has_target = false)]
		[ReturnsModifiedPointer ()]
		public void sort(size_t n, CompareFunc fn);

		[ReturnsModifiedPointer ()]
		public void remove(G data, CompareFunc fn, out G removed_data );

//~ 		/** free this but not the internal data */
//~ 		public void free();
		/** free the internal data of this */
		public void free_inner(FreeFunc fn);
//~ 		/** free this AND the internal data of this */
//~ 		[CCode (cname = "FREELIST")]
//~ 		[DestroysInstance]
//~ 		public void free_full ();

		public List<unowned G> copy();
		public List<G> copy_data();

		[ReturnsModifiedPointer ()]
		public void reverse ();

		/* item accessors */
		public unowned List<G> first();
		public unowned List<G> last();
		public unowned List<G> nth(size_t index);

		/* misc */
		public unowned G? find(G needle, CompareFunc fn);
		public unowned string? find_str(string needle);

		/** @return a list containing all items in `this` not present in `list` */
		public List<unowned G> diff(List<G>? list, CompareFunc fn);
	}
}

/* vim: set ts=2 sw=2 noet: */
