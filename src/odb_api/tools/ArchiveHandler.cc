/*
 * (C) Copyright 1996-2013 ECMWF.
 * 
 * This software is licensed under the terms of the Apache Licence Version 2.0
 * which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
 * In applying this licence, ECMWF does not waive the privileges and immunities 
 * granted to it by virtue of its status as an intergovernmental organisation nor
 * does it submit to any jurisdiction.
 */

#include <sstream>
#include <algorithm>
#include <string>

#include "eckit/io/FileHandle.h"
#include "eckit/parser/Request.h"
#include "eckit/parser/RequestParser.h"
#include "eckit/utils/ExecutionContext.h"
#include "eckit/utils/DataHandleFactory.h"

#include "ArchiveHandler.h"

#include "odb_api/FastODA2Request.h"

using namespace std;
using namespace eckit;

// TODO: clean it up!
const char * cfg_ = 
"CLASS: class\n"
"DATE: andate\n"
"TIME: antime\n"
"TYPE: type\n"
"OBSGROUP: groupid\n"
"REPORTYPE: reportype\n"
"STREAM: stream\n"
"EXPVER: expver\n"
;

ArchiveHandler::ArchiveHandler(const string& name) : RequestHandler(name) {}

Values ArchiveHandler::handle(const Request request)
{
    const string host (database(request));

    vector<string> sources(getValueAsList(request, "source"));
    if (! sources.size())
        throw UserError("You must specify file(s) to be archived using the SOURCE keyword");

    Values r(0);
    List list(r);
    for (size_t i(0); i < sources.size(); ++i)
    {
        const string source(sources[i]);

        Request generatedRequest (generateRequest(source));
        generatedRequest->value("database", host);
        checkRequestMatchesFilesMetaData(request, generatedRequest);
 
        Log::info() << "Request generated for file " << source << ":" << endl
                    << generatedRequest << endl;

        archive(source, host, request);

        list.append(string("mars://") + generatedRequest->str());
    }

    return r;
}

void ArchiveHandler::checkRequestMatchesFilesMetaData(const Request request, const Request generatedRequest)
{
    /*
    for (Request::const_iterator it(generatedRequest.begin()); it != generatedRequest.end(); ++it)
    {
        string key (it->first);
        Values values (it->second);

        Log::info() << "check if request matches files metadata: key=" << key << " value=" << values << endl;

        if (request.find(key) == request.end() || ! request.at(key).size())
        {
            string message("keyword '" + key + "' was not set in user's request");
            //throw UserError(message);
            Log::warning() << message << endl;
        }
        else
        {
            // TODO: check values in range
        }
    }
    */
}

Request ArchiveHandler::generateRequest(const string& source)
{
    odb::FastODA2Request<odb::ODA2RequestClientTraits> o2r;
    o2r.parseConfig(cfg_);

    eckit::OffsetList offsets;
    eckit::LengthList lengths;
    std::vector<odb::ODAHandle*> handles;
    bool rc (o2r.scanFile(source, offsets, lengths, handles));
    for (size_t i (0); i < handles.size(); ++i)
        delete handles[i];
    handles.clear();
    if (! rc)
        throw UserError(string("Cannot archive file ") + source);
    Log::info() << "File " << source << " has " << o2r.rowsNumber() << " rows." << endl;

    ASSERT(lengths.size() && lengths.size() == offsets.size());

    string r ( o2r.genRequest() );
    std::replace(r.begin(), r.end(), '\n', ' ');

    r.erase(std::remove(r.begin(), r.end(), ' '), r.end());
    
    r = "RETRIEVE," + r;
    Request requests (RequestParser::parse(r));
    //ASSERT(requests.size() == 1);
    //return requests.front();
    ASSERT(requests->tag() == "_requests");

    //requests->showGraph(string("generateRequest: ") + requests->str() );
    
    return requests->value();
}

void ArchiveHandler::archive(const PathName& source, const string& host, const Request request)
{
    Log::info() << "ARCHIVE " << source << " on " << host << endl;
    Log::info() << "ARCHIVE request: " << request << endl;

    FileHandle input(source);

    stringstream ss;
    ss << "mars://" << request;
    auto_ptr<DataHandle> mars (DataHandleFactory::openForWrite(ss.str()));

    input.saveInto(*mars);
}

/// If source not set then set its value with values taken from the stack
Values ArchiveHandler::handle(const Request request, ExecutionContext& context)
{
    //request->showGraph("ArchiveHandler::handle => " + request->str());
    Request req(request);
    popIfNotSet("source", req, context);
    Values r(handle(req));
    context.stack().push(r);
    //r->showGraph("ArchiveHandler::handle => " + r->str());
    return r;
}

