#pragma once

#include "json.h"
#include "json_reader.h"

class RequestHandler {

public:
	
	RequestHandler(reading_queries::JSONRequestBuilder& request_responder);
	void OutRequests(const json::Dict& map_requests, std::ostream& output) const;

private:

	reading_queries::JSONRequestBuilder& request_responder_;
};

