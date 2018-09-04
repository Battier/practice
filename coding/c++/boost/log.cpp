/*
 *          Copyright Andrey Semashev 2007 - 2015.
 * Distributed under the Boost Software License, Version 1.0.
 *    (See accompanying file LICENSE_1_0.txt or copy at
 *          http://www.boost.org/LICENSE_1_0.txt)
 */
/*!
 * \file   main.cpp
 * \author Andrey Semashev
 * \date   10.06.2008
 *
 * \brief  An example of logging in multiple threads.
 *         See the library tutorial for expanded comments on this code.
 *         It may also be worthwhile reading the Wiki requirements page:
 *         http://www.crystalclearsoftware.com/cgi-bin/boost_wiki/wiki.pl?Boost.Logging
 */

#define BOOST_LOG_DYN_LINK 1
#define BOOST_ALL_DYN_LINK 1

#include <stdexcept>
#include <string>
#include <iostream>
#include <fstream>
#include <boost/ref.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <boost/thread/barrier.hpp>
#include <boost/log/utility/setup/console.hpp>
#include <boost/log/common.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/attributes.hpp>
#include <boost/log/sinks.hpp>
#include <boost/log/sources/logger.hpp>
#include <boost/log/trivial.hpp>

#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/log/support/date_time.hpp>

namespace logging = boost::log;
namespace attrs = boost::log::attributes;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(my_logger, src::logger_mt)
        
using boost::shared_ptr;

enum
{
    LOG_RECORDS_TO_WRITE = 10000,
    THREAD_COUNT = 4
};

BOOST_LOG_INLINE_GLOBAL_LOGGER_DEFAULT(test_lg, src::logger_mt)

//! This function is executed in multiple threads
static void thread_fun(boost::barrier& bar)
{
    // Wait until all threads are created
    bar.wait();

    // Now, do some logging
    for (unsigned int i = 0; i < LOG_RECORDS_TO_WRITE; ++i)
    {
        BOOST_LOG(test_lg::get()) << "Log record " << i;
    }
}

BOOST_LOG_ATTRIBUTE_KEYWORD(line_id, "LineID", unsigned int)

static void init()
{
    logging::add_common_attributes();
    
    logging::add_file_log
        (
         keywords::file_name = "test_%Y-%m-%d_%H-%M-%S.%N.log",
         keywords::rotation_size = 10 * 1024 * 1024,
         keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),
         keywords::auto_flush = true,
         keywords::format = (
             expr::stream
             << expr::format_date_time< boost::posix_time::ptime >("TimeStamp", "%Y-%m-%d, %H:%M:%S.%f")
             << ": [" << std::setw(6) << std::setfill('0') << line_id << std::setfill(' ')
             << "]: <" << std::setw(8) << std::setiosflags(std::ios::left) << std::setfill(' ') << logging::trivial::severity
             << "> " << expr::smessage
             ) 
        );
    
//  logging::core::get()->set_filter
//  (
//      logging::trivial::severity >= logging::trivial::trace
//  );
}

static void logging_function2()
{
    src::logger_mt& lg = my_logger::get();
    BOOST_LOG(lg) << "Greetings from the global logger!";
}

int main(int argc, char* argv[])
{
  //  init();
//    logging::add_console_log();    
  //  logging::add_common_attributes();    
  //  logging_function2();

    BOOST_LOG_TRIVIAL(trace) << "A trace severity message";
    BOOST_LOG_TRIVIAL(debug) << "A debug severity message";
    BOOST_LOG_TRIVIAL(info) << "An informational severity message";
    BOOST_LOG_TRIVIAL(warning) << "A warning severity message";
    BOOST_LOG_TRIVIAL(error) << "An error severity message";
    BOOST_LOG_TRIVIAL(fatal) << "A fatal severity message";

#if 0
    try
    {
        // Open a rotating text file
        shared_ptr< std::ostream > strm(new std::ofstream("test.log"));
        if (!strm->good())
            throw std::runtime_error("Failed to open a text log file");

        // Create a text file sink
        shared_ptr< sinks::synchronous_sink< sinks::text_ostream_backend > > sink(
            new sinks::synchronous_sink< sinks::text_ostream_backend >);

        sink->locked_backend()->add_stream(strm);

        sink->set_formatter
        (
            expr::format("%1%: [%2%] [%3%] - %4%")
                % expr::attr< unsigned int >("RecordID")
                % expr::attr< boost::posix_time::ptime >("TimeStamp")
                % expr::attr< attrs::current_thread_id::value_type >("ThreadID")
                % expr::smessage
        );

        // Add it to the core
        logging::core::get()->add_sink(sink);

        // Add some attributes too
        logging::core::get()->add_global_attribute("TimeStamp", attrs::local_clock());
        logging::core::get()->add_global_attribute("RecordID", attrs::counter< unsigned int >());
        logging::core::get()->add_global_attribute("ThreadID", attrs::current_thread_id());

        // Create logging threads
        boost::barrier bar(THREAD_COUNT);
        boost::thread_group threads;
        for (unsigned int i = 0; i < THREAD_COUNT; ++i)
            threads.create_thread(boost::bind(&thread_fun, boost::ref(bar)));

        // Wait until all action ends
        threads.join_all();

        return 0;
    }
    catch (std::exception& e)
    {
        std::cout << "FAILURE: " << e.what() << std::endl;
        return 1;
    }
#endif
}
