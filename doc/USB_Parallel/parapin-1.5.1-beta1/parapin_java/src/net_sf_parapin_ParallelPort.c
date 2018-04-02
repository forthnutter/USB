#include <jni.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <parapindriver.h>
#include "net_sf_parapin_ParallelPort.h"

JNIEXPORT jint JNICALL Java_net_sf_parapin_ParallelPort_open0
  (JNIEnv *jnienv, jclass class)
{
    return (jint) open( "/dev/parapin", 0 );
}

JNIEXPORT void JNICALL Java_net_sf_parapin_ParallelPort_configurePin0
  (JNIEnv *jnienv, jclass class, jint port, jint pin, jbyte direction)
{
    ioctl( port, direction ? PPDRV_IOC_PINMODE_OUT : PPDRV_IOC_PINMODE_IN, LP_PIN[pin] );
}

JNIEXPORT jbyte JNICALL Java_net_sf_parapin_ParallelPort_logicLevelAtPin0
  (JNIEnv *jnienv, jclass class, jint port, jint pin)
{
    // the ioctl returns 0 for LOW and 65536 for HIGH
    return (jbyte) (ioctl( port, PPDRV_IOC_PINGET, LP_PIN[pin] ) >> 16);
}

JNIEXPORT void JNICALL Java_net_sf_parapin_ParallelPort_drivePin0
  (JNIEnv *jnienv, jclass class, jint port, jint pin, jbyte logicLevel)
{
    ioctl( port, logicLevel ? PPDRV_IOC_PINSET : PPDRV_IOC_PINCLEAR, LP_PIN[pin] );
}

JNIEXPORT void JNICALL Java_net_sf_parapin_ParallelPort_close0
  (JNIEnv *jnienv, jclass class, jint port)
{
    close( port );
}

