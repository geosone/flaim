/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class xflaim_DOMNode */

#ifndef _Included_xflaim_DOMNode
#define _Included_xflaim_DOMNode
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     xflaim_DOMNode
 * Method:    _createNode
 * Signature: (JJIIIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1createNode
  (JNIEnv *, jobject, jlong, jlong, jint, jint, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _createChildElement
 * Signature: (JJIZJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1createChildElement
  (JNIEnv *, jobject, jlong, jlong, jint, jboolean, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _deleteNode
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1deleteNode
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _deleteChildren
 * Signature: (JJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1deleteChildren
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNodeType
 * Signature: (J)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getNodeType
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _canHaveData
 * Signature: (J)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1canHaveData
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _createAttribute
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1createAttribute
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getFirstAttribute
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getFirstAttribute
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLastAttribute
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getLastAttribute
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAttribute
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getAttribute
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _deleteAttribute
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1deleteAttribute
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasAttribute
 * Signature: (JJI)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasAttribute
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasAttributes
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasAttributes
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasChildren
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasChildren
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasNextSibling
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasNextSibling
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasPreviousSibling
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasPreviousSibling
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _isNamespaceDecl
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1isNamespaceDecl
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getDocumentNode
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getDocumentNode
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getParentNode
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getParentNode
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getFirstChild
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getFirstChild
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLastChild
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getLastChild
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getChild
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getChild
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getPreviousSibling
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getPreviousSibling
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNextSibling
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getNextSibling
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getPreviousDocument
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getPreviousDocument
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNextDocument
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getNextDocument
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getPrefix
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getPrefix
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getPrefixId
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getPrefixId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getEncDefId
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getEncDefId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setPrefix
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setPrefix
  (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setPrefixId
 * Signature: (JJI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setPrefixId
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getChildElement
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getChildElement
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getSiblingElement
 * Signature: (JJIZJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getSiblingElement
  (JNIEnv *, jobject, jlong, jlong, jint, jboolean, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAncestorElement
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getAncestorElement
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getDescendantElement
 * Signature: (JJIJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getDescendantElement
  (JNIEnv *, jobject, jlong, jlong, jint, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getParentId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getParentId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNodeId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getNodeId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getDocumentId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getDocumentId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getPrevSibId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getPrevSibId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNextSibId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getNextSibId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getFirstChildId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getFirstChildId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLastChildId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getLastChildId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getFirstAttrId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getFirstAttrId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLastAttrId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getLastAttrId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNameId
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getNameId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getNamespaceURI
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getNamespaceURI
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLocalName
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getLocalName
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getQualifiedName
 * Signature: (JJ)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getQualifiedName
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getCollection
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getCollection
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getLong
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getLong
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAttributeValueLong
 * Signature: (JJIZJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getAttributeValueLong
  (JNIEnv *, jobject, jlong, jlong, jint, jboolean, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getString
 * Signature: (JJII)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getString
  (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAttributeValueString
 * Signature: (JJI)Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_xflaim_DOMNode__1getAttributeValueString
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getStringLen
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getStringLen
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getDataType
 * Signature: (JJ)I
 */
JNIEXPORT jint JNICALL Java_xflaim_DOMNode__1getDataType
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getDataLength
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getDataLength
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getBinary
 * Signature: (JJII)[B
 */
JNIEXPORT jbyteArray JNICALL Java_xflaim_DOMNode__1getBinary
  (JNIEnv *, jobject, jlong, jlong, jint, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAttributeValueBinary
 * Signature: (JJI)[B
 */
JNIEXPORT jbyteArray JNICALL Java_xflaim_DOMNode__1getAttributeValueBinary
  (JNIEnv *, jobject, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setLong
 * Signature: (JJJI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setLong
  (JNIEnv *, jobject, jlong, jlong, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setAttributeValueLong
 * Signature: (JJIJI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setAttributeValueLong
  (JNIEnv *, jobject, jlong, jlong, jint, jlong, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setString
 * Signature: (JJLjava/lang/String;ZI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setString
  (JNIEnv *, jobject, jlong, jlong, jstring, jboolean, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setAttributeValueString
 * Signature: (JJILjava/lang/String;I)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setAttributeValueString
  (JNIEnv *, jobject, jlong, jlong, jint, jstring, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setBinary
 * Signature: (JJ[BZI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setBinary
  (JNIEnv *, jobject, jlong, jlong, jbyteArray, jboolean, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setAttributeValueBinary
 * Signature: (JJI[BI)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setAttributeValueBinary
  (JNIEnv *, jobject, jlong, jlong, jint, jbyteArray, jint);

/*
 * Class:     xflaim_DOMNode
 * Method:    _createAnnotation
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1createAnnotation
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAnnotation
 * Signature: (JJJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getAnnotation
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getAnnotationId
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getAnnotationId
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _hasAnnotation
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1hasAnnotation
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _release
 * Signature: (J)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1release
  (JNIEnv *, jobject, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    setPrefix
 * Signature: (JJLjava/lang/String;)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode_setPrefix
  (JNIEnv *, jobject, jlong, jlong, jstring);

/*
 * Class:     xflaim_DOMNode
 * Method:    _getMetaValue
 * Signature: (JJ)J
 */
JNIEXPORT jlong JNICALL Java_xflaim_DOMNode__1getMetaValue
  (JNIEnv *, jobject, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _setMetaValue
 * Signature: (JJJ)V
 */
JNIEXPORT void JNICALL Java_xflaim_DOMNode__1setMetaValue
  (JNIEnv *, jobject, jlong, jlong, jlong);

/*
 * Class:     xflaim_DOMNode
 * Method:    _isDataLocalToNode
 * Signature: (JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_xflaim_DOMNode__1isDataLocalToNode
  (JNIEnv *, jobject, jlong, jlong);

#ifdef __cplusplus
}
#endif
#endif
