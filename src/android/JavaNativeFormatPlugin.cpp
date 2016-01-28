/*
 * Copyright (C) 2011-2015 FBReader.ORG Limited <contact@fbreader.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 */

#include <AndroidUtil.h>
#include <JniEnvelope.h>
#include <ZLFileImage.h>
#include <FileEncryptionInfo.h>

#include "zlibrary/core/filesystem/ZLAndroidFSManager.h"

#include "../common/fbreader/bookmodel/BookModel.h"
#include "../common/fbreader/bookmodel/ModelWriter.h"
#include "../common/fbreader/formats/FormatPlugin.h"
#include "../common/fbreader/library/Author.h"
#include "../common/fbreader/library/Book.h"
#include "../common/fbreader/library/Tag.h"
#include "../common/fbreader/library/UID.h"

static shared_ptr<FormatPlugin> findCppPlugin(jobject base) {
	const std::string fileType = AndroidUtil::Method_NativeFormatPlugin_supportedFileType->callForCppString(base);
	return PluginCollection::Instance().pluginByType(fileType);
}

static void fillUids(JNIEnv* env, jobject javaBook, Book &book) {
	const UIDList &uids = book.uids();
	for (UIDList::const_iterator it = uids.begin(); it != uids.end(); ++it) {
		JString type(env, (*it)->Type);
		JString id(env, (*it)->Id);
		AndroidUtil::Method_Book_addUid->call(javaBook, type.j(), id.j());
	}
}

static void fillMetaInfo(JNIEnv* env, jobject javaBook, Book &book) {
	JString title(env, book.title());
	AndroidUtil::Method_Book_setTitle->call(javaBook, title.j());

	JString language(env, book.language());
	if (language.j() != 0) {
		AndroidUtil::Method_Book_setLanguage->call(javaBook, language.j());
	}

	JString encoding(env, book.encoding());
	if (encoding.j() != 0) {
		AndroidUtil::Method_Book_setEncoding->call(javaBook, encoding.j());
	}

	JString seriesTitle(env, book.seriesTitle());
	if (seriesTitle.j() != 0) {
		JString indexString(env, book.indexInSeries());
		AndroidUtil::Method_Book_setSeriesInfo->call(javaBook, seriesTitle.j(), indexString.j());
	}

	const AuthorList &authors = book.authors();
	for (std::size_t i = 0; i < authors.size(); ++i) {
		const Author &author = *authors[i];
		JString name(env, author.name(), false);
		JString key(env, author.sortKey(), false);
		AndroidUtil::Method_Book_addAuthor->call(javaBook, name.j(), key.j());
	}

	const TagList &tags = book.tags();
	for (std::size_t i = 0; i < tags.size(); ++i) {
		JString jTag(env, tags[i]->fullName(), false);
		AndroidUtil::Method_Book_addTag->call(javaBook, jTag.j());
	}

	fillUids(env, javaBook, book);
}

static void fillLanguageAndEncoding(JNIEnv* env, jobject javaBook, Book &book) {
	JString language(env, book.language());
	if (language.j() != 0) {
		AndroidUtil::Method_Book_setLanguage->call(javaBook, language.j());
	}

	JString encoding(env, book.encoding());
	if (encoding.j() != 0) {
		AndroidUtil::Method_Book_setEncoding->call(javaBook, encoding.j());
	}
}

extern "C"
JNIEXPORT jint JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readMetainfoNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 1;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);

	if (!plugin->readMetainfo(*book)) {
		return 2;
	}

	fillMetaInfo(env, javaBook, *book);
	return 0;
}

extern "C"
JNIEXPORT jobject JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readEncryptionInfosNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 0;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	std::vector<shared_ptr<FileEncryptionInfo> > infos = plugin->readEncryptionInfos(*book);
	if (infos.empty()) {
		return 0;
	}

	jobjectArray jList = env->NewObjectArray(
		infos.size(), AndroidUtil::Class_FileEncryptionInfo.j(), 0
	);
	for (std::size_t i = 0; i < infos.size(); ++i) {
		jobject jInfo = AndroidUtil::createJavaEncryptionInfo(env, infos[i]);
		env->SetObjectArrayElement(jList, i, jInfo);
		env->DeleteLocalRef(jInfo);
	}
	return jList;
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readUidsNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);

	plugin->readUids(*book);
	fillUids(env, javaBook, *book);
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_detectLanguageAndEncodingNative(JNIEnv* env, jobject thiz, jobject javaBook) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	if (!plugin->readLanguageAndEncoding(*book)) {
		return;
	}

	fillLanguageAndEncoding(env, javaBook, *book);
}

extern "C"
JNIEXPORT jint JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readModelNative(JNIEnv* env, jobject thiz, jobject javaBook, jobject fileHandler) {
	ZLAndroidFSManager::setFileHandler(fileHandler);

	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 1;
	}

	jstring javaCacheDir = (jstring)AndroidUtil::Field_SafeFileHandler_Dir->value(fileHandler);
	const std::string cacheDir = AndroidUtil::fromJavaString(env, javaCacheDir);
	env->DeleteLocalRef(javaCacheDir);

	shared_ptr<Book> book = AndroidUtil::bookFromJavaBook(env, javaBook);
	shared_ptr<BookModel> model = new BookModel(book, cacheDir);
	if (!plugin->readModel(*model)) {
		return 2;
	}
	if (!model->flush()) {
		return 3;
	}

	ModelWriter writer(cacheDir);
	writer.writeModelInfo(*model);

	ZLAndroidFSManager::setFileHandler(0);

	return 0;
}

extern "C"
JNIEXPORT jstring JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readAnnotationNative(JNIEnv* env, jobject thiz, jobject file) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return 0;
	}

	const std::string path = AndroidUtil::Method_ZLFile_getPath->callForCppString(file);
	return AndroidUtil::createJavaString(env, plugin->readAnnotation(ZLFile(path)));
}

extern "C"
JNIEXPORT void JNICALL Java_org_geometerplus_fbreader_formats_NativeFormatPlugin_readCoverNative(JNIEnv* env, jobject thiz, jobject file, jobjectArray box) {
	shared_ptr<FormatPlugin> plugin = findCppPlugin(thiz);
	if (plugin.isNull()) {
		return;
	}

	const std::string path = AndroidUtil::Method_ZLFile_getPath->callForCppString(file);

	shared_ptr<const ZLImage> image = plugin->coverImage(ZLFile(path));
	if (!image.isNull()) {
		jobject javaImage = AndroidUtil::createJavaImage(env, (const ZLFileImage&)*image);
		env->SetObjectArrayElement(box, 0, javaImage);
		env->DeleteLocalRef(javaImage);
	}
}
