#include <iostream>
#include <memory>
#include <uriparser/Uri.h>
#include <EdUrlParser.h>
#include <http_parser.h>

#include "ada.h"
#include "performancecounters/event_counter.h"
event_collector collector;
size_t N = 1000;

#include <benchmark/benchmark.h>