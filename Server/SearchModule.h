#pragma once

/**
 * An interface for search modules that can be used by an Optimizer instance to
 * search for new tuning parameter values that might improve the client's
 * performance.
 */
class SearchModule {
	/**
	 * Searches for new tuning parameter values.
	 * 
	 * @return the status of the search
	 */
	virtual int doSearch() = 0;	
};