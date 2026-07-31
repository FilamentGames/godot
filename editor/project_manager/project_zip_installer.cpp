/**************************************************************************/
/*  project_zip_installer.cpp                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "project_zip_installer.h"

#include "core/io/dir_access.h"
#include "core/io/file_access.h"
#include "core/io/zip_io.h"

Error install_project_from_zip(const String &p_zip_path, const String &p_target_dir, bool p_create_dir, Vector<String> *r_failed_files) {
	ERR_FAIL_COND_V(p_zip_path.is_empty(), ERR_INVALID_PARAMETER);
	ERR_FAIL_COND_V(p_target_dir.is_empty(), ERR_INVALID_PARAMETER);

	Ref<FileAccess> io_fa;
	zlib_filefunc_def io = zipio_create_io(&io_fa);

	unzFile pkg = unzOpen2(p_zip_path.utf8().get_data(), &io);
	if (!pkg) {
		return ERR_FILE_CORRUPT;
	}

	// Find the first directory with a "project.godot".
	String zip_root;
	int ret = unzGoToFirstFile(pkg);
	while (ret == UNZ_OK) {
		unz_file_info info;
		char fname[16384];
		ret = unzGetCurrentFileInfo(pkg, &info, fname, 16384, nullptr, 0, nullptr, 0);
		if (ret != UNZ_OK) {
			unzClose(pkg);
			return FAILED;
		}

		String name = String::utf8(fname);

		// Skip the __MACOSX directory created by macOS's built-in file zipper.
		if (name.begins_with("__MACOSX")) {
			ret = unzGoToNextFile(pkg);
			continue;
		}

		if (name.get_file() == "project.godot") {
			zip_root = name.get_base_dir();
			break;
		}

		ret = unzGoToNextFile(pkg);
	}

	if (ret == UNZ_END_OF_LIST_OF_FILE) {
		unzClose(pkg);
		return ERR_FILE_NOT_FOUND;
	}

	if (p_create_dir) {
		Ref<DirAccess> d = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
		if (!d->dir_exists(p_target_dir) && d->make_dir(p_target_dir) != OK) {
			unzClose(pkg);
			return ERR_CANT_CREATE;
		}
	}

	ret = unzGoToFirstFile(pkg);

	Vector<String> failed_files;
	while (ret == UNZ_OK) {
		unz_file_info info;
		char fname[16384];
		ret = unzGetCurrentFileInfo(pkg, &info, fname, 16384, nullptr, 0, nullptr, 0);
		if (ret != UNZ_OK) {
			unzClose(pkg);
			return FAILED;
		}

		String name = String::utf8(fname);

		// Skip the __MACOSX directory created by macOS's built-in file zipper.
		if (name.begins_with("__MACOSX")) {
			ret = unzGoToNextFile(pkg);
			continue;
		}

		String rel_path = name.trim_prefix(zip_root);
		if (rel_path.is_empty()) { // Root.
		} else if (rel_path.ends_with("/")) { // Directory.
			Ref<DirAccess> da = DirAccess::create(DirAccess::ACCESS_FILESYSTEM);
			da->make_dir(p_target_dir.path_join(rel_path));
		} else { // File.
			Vector<uint8_t> uncomp_data;
			uncomp_data.resize(info.uncompressed_size);

			unzOpenCurrentFile(pkg);
			ret = unzReadCurrentFile(pkg, uncomp_data.ptrw(), uncomp_data.size());
			if (ret < 0) {
				unzCloseCurrentFile(pkg);
				unzClose(pkg);
				return FAILED;
			}
			unzCloseCurrentFile(pkg);

			Ref<FileAccess> f = FileAccess::open(p_target_dir.path_join(rel_path), FileAccess::WRITE);
			if (f.is_valid()) {
				f->store_buffer(uncomp_data.ptr(), uncomp_data.size());
			} else {
				failed_files.push_back(rel_path);
			}
		}

		ret = unzGoToNextFile(pkg);
	}

	unzClose(pkg);

	if (failed_files.size()) {
		if (r_failed_files) {
			*r_failed_files = failed_files;
		}
		return FAILED;
	}

	return OK;
}
