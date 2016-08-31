/*
 *  alpm_octopi_utils
 *
 *  Copyright (C) 2014-2016 Guillaume Benoit <guillaume@manjaro.org>
 *  Copyright (C) 2016 Alexandre Albuquerque Arnt
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 3 of the License, or
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

int alpm_pkg_compare_name (Alpm.Package pkg_a, Alpm.Package pkg_b) {
	return strcmp (pkg_a.name, pkg_b.name);
}

[Compact]
public class AlpmUtils {
	public string conf_file_path;
	public Alpm.Handle alpm_handle;
	public Alpm.List<string> holdpkgs;
	public Alpm.List<string> syncfirsts;

	public AlpmUtils (string conf_file_path) {
		this.conf_file_path = conf_file_path;
		refresh_handle ();
	}

	public void refresh_handle () {
		var alpm_config = new AlpmConfig (conf_file_path);
		alpm_handle = alpm_config.get_handle ();
		holdpkgs = (owned) alpm_config.holdpkgs;
		syncfirsts = (owned) alpm_config.syncfirsts;
	}

	public unowned Alpm.Package? get_installed_pkg (string pkg_name) {
		return alpm_handle.localdb.get_pkg (pkg_name);
	}

	public unowned Alpm.Package? get_sync_pkg (string pkg_name) {
		unowned Alpm.Package? pkg = null;
		unowned Alpm.List<unowned Alpm.DB> list = alpm_handle.syncdbs;
		while (list != null) {
			unowned Alpm.DB db = list.data;
			pkg = db.get_pkg (pkg_name);
			if (pkg != null) {
				break;
			}
			list = list.next;
		}
		return pkg;
	}

	public Alpm.List<unowned Alpm.Package> search_all_dbs (string search_string) {
		Alpm.List<unowned Alpm.Package> syncpkgs = null;
		Alpm.List<unowned string> needles = null;
		string[] splitted = search_string.split (" ");
		foreach (unowned string part in splitted) {
			needles.append (part);
		}
		Alpm.List<unowned Alpm.Package> results = alpm_handle.localdb.search (needles);
		unowned Alpm.List<unowned Alpm.DB> list = alpm_handle.syncdbs;
		while (list != null) {
			unowned Alpm.DB db = list.data;
			if (syncpkgs.length () == 0) {
				syncpkgs = db.search (needles);
			} else {
				//syncpkgs.join (db.search (needles).diff (syncpkgs, (Alpm.List.CompareFunc) alpm_pkg_compare_name));
				syncpkgs.join (db.search (needles));
			}
			list = list.next;
		}
		//results.join (syncpkgs.diff (results, (Alpm.List.CompareFunc) alpm_pkg_compare_name));
                
		results.join (syncpkgs.copy());
		//results.sort ((Alpm.List.CompareFunc) alpm_pkg_compare_name);
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_group_pkgs (string group_name) {
		Alpm.List<unowned Alpm.Package> results = null;
		unowned Alpm.Group? grp = alpm_handle.localdb.get_group (group_name);
		if (grp != null) {
			unowned Alpm.List<unowned Alpm.Package> list = grp.packages;
			while (list != null) {
				unowned Alpm.Package pkg = list.data;
				results.append (pkg);
				list = list.next;
			}
		}
		results.join (Alpm.find_group_pkgs (alpm_handle.syncdbs, group_name).diff (results, (Alpm.List.CompareFunc) alpm_pkg_compare_name));
		//results.sort ((Alpm.List.CompareFunc) alpm_pkg_compare_name);
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_installed_pkgs () {
		return alpm_handle.localdb.pkgcache.copy ();
	}

	public Alpm.List<unowned Alpm.Package> get_orphans () {
		Alpm.List<unowned Alpm.Package> results = null;
		unowned Alpm.List<unowned Alpm.Package> list = alpm_handle.localdb.pkgcache;
		while (list != null) {
			unowned Alpm.Package pkg = list.data;
			if (pkg.reason == Alpm.Package.Reason.DEPEND) {
				Alpm.List<string> requiredby = pkg.compute_requiredby ();
				if (requiredby.length () == 0) {
					Alpm.List<string> optionalfor = pkg.compute_optionalfor ();
					if (optionalfor.length () == 0) {
						results.append (pkg);
					}
					optionalfor.free_inner (GLib.free);
				}
				requiredby.free_inner (GLib.free);
			}
			list = list.next;
		}
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_unrequired () {
		Alpm.List<unowned Alpm.Package> results = null;
		unowned Alpm.List<unowned Alpm.Package> list = alpm_handle.localdb.pkgcache;
		while (list != null) {
			unowned Alpm.Package pkg = list.data;
			Alpm.List<string> requiredby = pkg.compute_requiredby ();
			if (requiredby.length () == 0) {
				Alpm.List<string> optionalfor = pkg.compute_optionalfor ();
				if (optionalfor.length () == 0) {
					results.append (pkg);
				}
				optionalfor.free_inner (GLib.free);
			}
			requiredby.free_inner (GLib.free);
			list = list.next;
		}
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_foreign_pkgs () {
		Alpm.List<unowned Alpm.Package> results = null;
		unowned Alpm.List<unowned Alpm.Package> list = alpm_handle.localdb.pkgcache;
		while (list != null) {
			unowned Alpm.Package pkg = list.data;
			if (get_sync_pkg (pkg.name) == null) {
				results.append (pkg);
			}
			list = list.next;
		}
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_repo_pkgs (string repo_name) {
		Alpm.List<unowned Alpm.Package> results = null;
		unowned Alpm.List<unowned Alpm.DB> list = alpm_handle.syncdbs;
		while (list != null) {
			unowned Alpm.DB db = list.data;
			if (db.name == repo_name) {
				unowned Alpm.List<unowned Alpm.Package> list2 = db.pkgcache;
				while (list2 != null) {
					unowned Alpm.Package sync_pkg = list2.data;
					unowned Alpm.Package?local_pkg = alpm_handle.localdb.get_pkg (sync_pkg.name);
					if (local_pkg != null) {
						results.append (local_pkg);
					} else {
						results.append (sync_pkg);
					}
					list2 = list2.next;
				}
			}
			list = list.next;
		}
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_all_pkgs () {
		Alpm.List<unowned Alpm.Package> syncpkgs = null;
		Alpm.List<unowned Alpm.Package> results = null;
		results = alpm_handle.localdb.pkgcache.copy ();
		unowned Alpm.List<unowned Alpm.DB> list = alpm_handle.syncdbs;
		while (list != null) {
			unowned Alpm.DB db = list.data;
			if (syncpkgs.length () == 0)
				syncpkgs = db.pkgcache.copy ();
			else {
				syncpkgs.join (db.pkgcache.diff (syncpkgs, (Alpm.List.CompareFunc) alpm_pkg_compare_name));
			}
			list = list.next;
		}
		results.join (syncpkgs.diff (results, (Alpm.List.CompareFunc) alpm_pkg_compare_name));
		//results.sort ((Alpm.List.CompareFunc) alpm_pkg_compare_name);
		return results;
	}

	public Alpm.List<unowned Alpm.Package> get_updates () {
			Alpm.List<unowned Alpm.Package> results = null;
			unowned Alpm.List<string> list = syncfirsts;
			while (list != null) {
				unowned string name = list.data;
				unowned Alpm.Package? pkg = Alpm.find_satisfier (alpm_handle.localdb.pkgcache, name);
				if (pkg != null) {
					unowned Alpm.Package? candidate = pkg.sync_newversion (alpm_handle.syncdbs);
					if (candidate != null) {
						results.append (candidate);
					}
				}
				list = list.next;
			}
			if (results.length () != 0) {
				return results;
			} else {
				unowned Alpm.List<unowned Alpm.Package> list2 = alpm_handle.localdb.pkgcache;
				while (list2 != null) {
					unowned Alpm.Package installed_pkg = list2.data;
					// check if installed_pkg is in IgnorePkg or IgnoreGroup
					if (alpm_handle.should_ignore (installed_pkg) == 0) {
						unowned Alpm.Package? candidate = installed_pkg.sync_newversion (alpm_handle.syncdbs);
						if (candidate != null) {
							results.append (candidate);
						}
					}
					list2 = list2.next;
				}
			}
			return results;
	}
}
