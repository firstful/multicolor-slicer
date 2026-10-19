#ifdef WIN32
	#ifndef WIN32_LEAN_AND_MEAN
		#define WIN32_LEAN_AND_MEAN
	#endif
	#ifndef NOMINMAX
		#define NOMINMAX
	#endif
#endif

#include <float.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>

#include <cassert>
#include <climits>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include <algorithm>
#include <atomic>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iostream>
#include <istream>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <numeric>
#include <ostream>
#include <queue>
#include <regex>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <string>
#include <thread>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include <boost/config.hpp>

// boost/property_tree/json_parser/detail/parser.hpp includes boost/bind.hpp, which is deprecated.
// Suppress the following boost message:
// The practice of declaring the Bind placeholders (_1, _2, ...) in the global namespace is deprecated.
#define BOOST_BIND_GLOBAL_PLACEHOLDERS
#undef BOOST_BIND_GLOBAL_PLACEHOLDERS

#include <boost/version.hpp>


#include <Eigen/Geometry>


#include "clipper.hpp"
#include "BoundingBox.hpp"
#include "ClipperUtils.hpp"
#include "Config.hpp"
#include "enum_bitmask.hpp"
#include "format.hpp"
#include "I18N.hpp"
#include "MultiPoint.hpp"
#include "Point.hpp"
#include "Polygon.hpp"
#include "Polyline.hpp"
#include "SVG.hpp"

#include "libslic3r.h"
#include "libslic3r_version.h"


