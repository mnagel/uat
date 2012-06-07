#include "Optimizer.h"
Optimizer::Optimizer(McHandler* mcHandler):
	mcHandler(mcHandler) {

}

Optimizer::~Optimizer() {

}

void Optimizer::setInitialConfig() {
	mcHandler->setConfigToMin();
}

void Optimizer::setNextConfig() {
	mcHandler->raiseConfig();
}
