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
 * Method:    _dbLock
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1dbLock
  (JNIEnv *, jobject, jlong, jint, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _dbUnlock
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1dbUnlock
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getLockType
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getLockType
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getLockImplicit
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_Db__1getLockImplicit
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getLockThreadId
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getLockThreadId
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getLockNumExclQueued
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getLockNumExclQueued
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getLockNumSharedQueued
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getLockNumSharedQueued
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getLockPriorityCount
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getLockPriorityCount
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _indexSuspend
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1indexSuspend
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _indexResume
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1indexResume
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _reduceSize
 * Signature: (JI)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1reduceSize
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _documentDone
 * Signature: (JIJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1documentDone__JIJ
  (JNIEnv *, jobject, jlong, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _documentDone
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1documentDone__JJ
  (JNIEnv *, jobject, jlong, jlong);

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
JNIEXPORT void JNICALL Java_xflaim_Db__1transBegin__JIII
  (JNIEnv *, jobject, jlong, jint, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _transBegin
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1transBegin__JJ
  (JNIEnv *, jobject, jlong, jlong);

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
 * Method:    _getTransType
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getTransType
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _doCheckpoint
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1doCheckpoint
  (JNIEnv *, jobject, jlong, jint);

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
 * Signature: (JIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getFirstDocument
  (JNIEnv *, jobject, jlong, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getLastDocument
 * Signature: (JIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getLastDocument
  (JNIEnv *, jobject, jlong, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getDocument
 * Signature: (JIIJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getDocument
  (JNIEnv *, jobject, jlong, jint, jint, jlong, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getNode
 * Signature: (JIJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getNode
  (JNIEnv *, jobject, jlong, jint, jlong, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getAttribute
 * Signature: (JIJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getAttribute
  (JNIEnv *, jobject, jlong, jint, jlong, jint, jlong);

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
 * Method:    _getElementNameId
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getElementNameId
  (JNIEnv *, jobject, jlong, jstring, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _createUniqueElmDef
 * Signature: (JLjava/lang/String;Ljava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createUniqueElmDef
  (JNIEnv *, jobject, jlong, jstring, jstring, jint);

/*
 * Class:     xflaim_Db
 * Method:    _createAttributeDef
 * Signature: (JLjava/lang/String;Ljava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createAttributeDef
  (JNIEnv *, jobject, jlong, jstring, jstring, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getAttributeNameId
 * Signature: (JLjava/lang/String;Ljava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getAttributeNameId
  (JNIEnv *, jobject, jlong, jstring, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _createPrefixDef
 * Signature: (JLjava/lang/String;I)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createPrefixDef
  (JNIEnv *, jobject, jlong, jstring, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getPrefixId
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getPrefixId
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _createEncDef
 * Signature: (JLjava/lang/String;Ljava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createEncDef
  (JNIEnv *, jobject, jlong, jstring, jstring, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getEncDefId
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getEncDefId
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _createCollectionDef
 * Signature: (JLjava/lang/String;II)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1createCollectionDef
  (JNIEnv *, jobject, jlong, jstring, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getCollectionNumber
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getCollectionNumber
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _getIndexNumber
 * Signature: (JLjava/lang/String;)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getIndexNumber
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _getDictionaryDef
 * Signature: (JIIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Db__1getDictionaryDef
  (JNIEnv *, jobject, jlong, jint, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _getDictionaryName
 * Signature: (JII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_Db__1getDictionaryName
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getElementNamespace
 * Signature: (JI)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_Db__1getElementNamespace
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getAttributeNamespace
 * Signature: (JI)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_Db__1getAttributeNamespace
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Db
 * Method:    _getDataType
 * Signature: (JII)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Db__1getDataType
  (JNIEnv *, jobject, jlong, jint, jint);

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

/*
 * Class:     xflaim_Db
 * Method:    _changeItemState
 * Signature: (JIILjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1changeItemState
  (JNIEnv *, jobject, jlong, jint, jint, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _getRflFileName
 * Signature: (JIZ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_Db__1getRflFileName
  (JNIEnv *, jobject, jlong, jint, jboolean);

/*
 * Class:     xflaim_Db
 * Method:    _setNextNodeId
 * Signature: (JIJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1setNextNodeId
  (JNIEnv *, jobject, jlong, jint, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _setNextDictNum
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1setNextDictNum
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     xflaim_Db
 * Method:    _setRflKeepFilesFlag
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1setRflKeepFilesFlag
  (JNIEnv *, jobject, jlong, jboolean);

/*
 * Class:     xflaim_Db
 * Method:    _getRflKeepFlag
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_Db__1getRflKeepFlag
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Db
 * Method:    _setRflDir
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_Db__1setRflDir
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Db
 * Method:    _getRflDir
 * Signature: (J)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_Db__1getRflDir
  (JNIEnv *, jobject, jlong);

#ifdef __cplusplus
}
#endif
#endif
