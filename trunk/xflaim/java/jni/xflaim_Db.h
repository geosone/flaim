/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class xflaim_Db */

#ifndef _Included_xflaim_Db
#define _Included_xflaim_Db
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     xflaim_Db
 * Method:    _release
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1release
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _transBegin
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1transBegin
  (JNIEnv *, jobject, jlong, jint, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _transCommit
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1transCommit
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _transAbort
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1transAbort
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _import
 * Signature: (JLxflaim/PosIStream;I)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1import
  (JNIEnv *, jobject, jlong, jobject, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getFirstDocument
 * Signature: (JILxflaim/DOMNode;)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getFirstDocument
  (JNIEnv *, jobject, jlong, jint, jobject);

/*
 * Class:     xflaim_Db
 * Method:    _getNode
 * Signature: (JIJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getNode
  (JNIEnv *, jobject, jlong, jint, jlong, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _createDocument
 * Signature: (JI)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1createDocument
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _createRootElement
 * Signature: (JII)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1createRootElement
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _createElementDef
 * Signature: (JLjava/lang/String;Ljava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createElementDef
  (JNIEnv *, jobject, jlong, jstring, jstring, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _backupBegin
 * Signature: (JIIIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1backupBegin
  (JNIEnv *, jobject, jlong, jint, jint, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _keyRetrieve
 * Signature: (JIJIJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1keyRetrieve
  (JNIEnv *, jobject, jlong, jint, jlong, jint, jlong);

#ifdef __cplusplus
}
#endif
#endif
