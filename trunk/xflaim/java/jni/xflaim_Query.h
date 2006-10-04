/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class xflaim_Query */

#ifndef _Included_xflaim_Query
#define _Included_xflaim_Query
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     xflaim_Query
 * Method:    _release
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1release
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _createQuery
 * Signature: (I)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1createQuery
  (JNIEnv *, jobject, jint);

/*
 * Class:     xflaim_Query
 * Method:    _setLanguage
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1setLanguage
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _setupQueryExpr
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1setupQueryExpr
  (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     xflaim_Query
 * Method:    _copyCriteria
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1copyCriteria
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _addXPathComponent
 * Signature: (JIII)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addXPathComponent
  (JNIEnv *, jobject, jlong, jint, jint, jint);

/*
 * Class:     xflaim_Query
 * Method:    _addOperator
 * Signature: (JII)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addOperator
  (JNIEnv *, jobject, jlong, jint, jint);

/*
 * Class:     xflaim_Query
 * Method:    _addStringValue
 * Signature: (JLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addStringValue
  (JNIEnv *, jobject, jlong, jstring);

/*
 * Class:     xflaim_Query
 * Method:    _addBinaryValue
 * Signature: (J[B)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addBinaryValue
  (JNIEnv *, jobject, jlong, jbyteArray);

/*
 * Class:     xflaim_Query
 * Method:    _addLongValue
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addLongValue
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _addBoolean
 * Signature: (JZZ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1addBoolean
  (JNIEnv *, jobject, jlong, jboolean, jboolean);

/*
 * Class:     xflaim_Query
 * Method:    _getFirst
 * Signature: (JJJI)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1getFirst
  (JNIEnv *, jobject, jlong, jlong, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getLast
 * Signature: (JJJI)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1getLast
  (JNIEnv *, jobject, jlong, jlong, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getNext
 * Signature: (JJJII)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1getNext
  (JNIEnv *, jobject, jlong, jlong, jlong, jint, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getPrev
 * Signature: (JJJII)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1getPrev
  (JNIEnv *, jobject, jlong, jlong, jlong, jint, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getCurrent
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1getCurrent
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _resetQuery
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1resetQuery
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _getStatsAndOptInfo
 * Signature: (J)[Lxflaim/OptInfo;
 */
JNIEXPORT jobjectArray JNICALL Java_xflaim_Query__1getStatsAndOptInfo
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _setDupHandling
 * Signature: (JZ)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1setDupHandling
  (JNIEnv *, jobject, jlong, jboolean);

/*
 * Class:     xflaim_Query
 * Method:    _setIndex
 * Signature: (JI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1setIndex
  (JNIEnv *, jobject, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getIndex
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Query__1getIndex
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _usesMultipleIndexes
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_Query__1usesMultipleIndexes
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _addSortKey
 * Signature: (JJZZIIIIZZ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1addSortKey
  (JNIEnv *, jobject, jlong, jlong, jboolean, jboolean, jint, jint, jint, jint, jboolean, jboolean);

/*
 * Class:     xflaim_Query
 * Method:    _enablePositioning
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1enablePositioning
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _positionTo
 * Signature: (JJJII)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1positionTo
  (JNIEnv *, jobject, jlong, jlong, jlong, jint, jint);

/*
 * Class:     xflaim_Query
 * Method:    _positionTo2
 * Signature: (JJJIJI)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_Query__1positionTo2
  (JNIEnv *, jobject, jlong, jlong, jlong, jint, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _getPosition
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_Query__1getPosition
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _buildResultSet
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1buildResultSet
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_Query
 * Method:    _stopBuildingResultSet
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1stopBuildingResultSet
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _enableResultSetEncryption
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_Query__1enableResultSetEncryption
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_Query
 * Method:    _getResultSetCounts
 * Signature: (JJIZ)Lxflaim/ResultSetCounts;
 */
JNIEXPORT jobject JNICALL Java_xflaim_Query__1getResultSetCounts
  (JNIEnv *, jobject, jlong, jlong, jint, jboolean);

#ifdef __cplusplus
}
#endif
#endif