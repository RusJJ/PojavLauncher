#include <jni.h>

#include "utils.h"

jclass inputBridgeClass;
jmethodID inputBridgeMethod;

JavaVM* firstJavaVM;
JNIEnv* firstJNIEnv;

JavaVM* secondJavaVM;
JNIEnv* secondJNIEnv;

void attachThreadIfNeed(bool* isAttached) {
    if (!*isAttached && secondJavaVM && secondJNIEnv) {
        (*secondJavaVM)->AttachCurrentThread(secondJavaVM, &secondJNIEnv, NULL);
        *isAttached = true;
    }
}

JNIEXPORT void JNICALL Java_org_lwjgl_glfw_CallbackBridge_sendData(JNIEnv* env, jclass clazz, jint type, jstring data) {
    if (&env == &dalvikJNIEnvPtr) {
        firstJavaVM = dalvikJavaVMPtr;
        firstJNIEnv = dalvikJNIEnvPtr;
        secondJavaVM = runtimeJavaVMPtr;
        secondJNIEnv = runtimeJNIEnvPtr;
        
        attachThreadIfNeed(&isAndroidThreadAttached);
    } else if (&env == &runtimeJNIEnvPtr) {
        firstJavaVM = runtimeJavaVMPtr;
        firstJNIEnv = runtimeJNIEnvPtr;
        secondJavaVM = dalvikJavaVMPtr;
        secondJNIEnv = dalvikJNIEnvPtr;
        
        attachThreadIfNeed(&isRuntimeThreadAttached);
    } else {
        printf("cancel input, env=%p, artenv=%p, %jvmenv=%p\n", &env, &dalvikJNIEnvPtr, &runtimeJNIEnvPtr);
        return;
    }
    
    printf("[SendData] type=%i, isJVMNull=%p\n", type, secondJavaVM == NULL);
    
    if (secondJavaVM != NULL) {
        char *data_c = (char*)(*env)->GetStringUTFChars(env, data, 0);
        printf("data=%s\n", data_c);
        jstring data_jre = (*secondJNIEnv)->NewStringUTF(secondJNIEnv, data_c);
        (*env)->ReleaseStringUTFChars(env, data, data_c);
    
        if (inputBridgeMethod == NULL) {
            inputBridgeClass = (*secondJNIEnv)->FindClass(secondJNIEnv, "org/lwjgl/glfw/CallbackBridge");
            inputBridgeMethod = (*secondJNIEnv)->GetStaticMethodID(secondJNIEnv, inputBridgeClass, "receiveCallback", "(ILjava/lang/String;)V");
        }
        (*secondJNIEnv)->CallStaticVoidMethod(secondJNIEnv, inputBridgeClass, inputBridgeMethod, type, data_jre);
    }
    // else: too early!
}
