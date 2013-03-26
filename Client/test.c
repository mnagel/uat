#include <stdio.h>
#include "tunerWrapper.h"

int main() {
        CTuner *t = NULL;

        t = ctuner_new();
	ctuner_delete(t);
        t = NULL;

        return 0;
}
