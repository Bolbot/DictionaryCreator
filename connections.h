#ifndef __CONNECTIONS_H__
#define __CONNECTIONS_H__

#include "utils.h"
#include <cpr/cpr.h>

namespace connections
{
	StringType get(const StringType &URI);

	StringType lookup_online_dictionary(const StringType &word);
}

#endif
