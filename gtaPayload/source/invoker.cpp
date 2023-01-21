#include "invoker.h"
#include <string.h>

NativeArg_s nativeArg;
u64 args[32];

void setVectors() {
	while (nativeArg.vectorCount) {
		nativeArg.vectorCount--;

		Vector3* argVector = nativeArg.argVectors[nativeArg.vectorCount];
		Vector4 tempVector = nativeArg.tempVectors[nativeArg.vectorCount];
		argVector->x = tempVector.x;
		argVector->y = tempVector.y;
		argVector->z = tempVector.z;
	}
}

void resetArgs() {
	nativeArg.argCount = 0;
	nativeArg.vectorCount = 0;
	nativeArg.argValues = nativeArg.returnValue = args;
	memset(args, 0, sizeof(args));
}
