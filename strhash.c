#include <stdio.h>
#include <assert.h>

#include "strhash.h"

hashcode_t strhash(cstring string) {
    hashcode_t hashcode = 0;
    cstring mover;
    assert(string != NULL);

    for(mover = string; *mover != '\0'; mover++) {
        hashcode = (hashcode << 5) - hashcode + (unsigned char)(*mover);
    }

    return hashcode;
}
