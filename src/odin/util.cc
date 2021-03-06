#include <boost/filesystem/operations.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string/replace.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/date_time/local_time/local_time.hpp>

#include <valhalla/midgard/logging.h>

#include "proto/directions_options.pb.h"
#include "odin/util.h"
#include "odin/narrative_dictionary.h"
#include "locales.h"

namespace {

  valhalla::odin::locales_singleton_t load_narrative_locals() {
    valhalla::odin::locales_singleton_t locales;
    //for each locale
    for(const auto& json : locales_json) {
      LOG_TRACE("LOCALES");
      LOG_TRACE("-------");
      LOG_TRACE("- " + json.first);
      //load the json
      boost::property_tree::ptree narrative_pt;
      std::stringstream ss; ss << json.second;
      boost::property_tree::read_json(ss, narrative_pt);
      LOG_TRACE("JSON read");
      //parse it into an object and add it to the map
      auto narrative_dictionary = std::make_shared<valhalla::odin::NarrativeDictionary>(narrative_pt);
      LOG_TRACE("NarrativeDictionary created");
      locales.insert(std::make_pair(json.first, narrative_dictionary));
      //insert all the aliases as this same object
      auto aliases = narrative_pt.get_child("aliases");
      for(const auto& alias : aliases) {
        auto name = alias.second.get_value<std::string>();
        auto inserted = locales.insert(std::make_pair(name, narrative_dictionary));
        if(!inserted.second) {
          throw std::logic_error("Alias '" + name + "' in json locale '" + json.first +
            "' has duplicate with posix_locale '" + inserted.first->second->GetLocale().name());
        }
      }
    }
    return locales;
  }

}


namespace valhalla {
namespace odin {

// Returns the specified item surrounded with quotes.
std::string GetQuotedString(const std::string& item) {
  std::string str;
  str += "\"";
  str += item;
  str += "\"";
  return str;
}

bool IsSimilarTurnDegree(uint32_t path_turn_degree,
                         uint32_t intersecting_turn_degree, bool is_right,
                         uint32_t turn_degree_threshold) {
  int32_t turn_degree_delta = 0;
  if (is_right) {
    turn_degree_delta = (((intersecting_turn_degree - path_turn_degree) + 360)
        % 360);
  } else {
    turn_degree_delta = (((path_turn_degree - intersecting_turn_degree) + 360)
        % 360);
  }

  return (turn_degree_delta <= turn_degree_threshold);
}

DirectionsOptions GetDirectionsOptions(const boost::property_tree::ptree& pt) {
  valhalla::odin::DirectionsOptions directions_options;

  // TODO: validate values coming soon...

  auto units_ptr = pt.get_optional<std::string>("units");
  if (units_ptr) {
    std::string units = *units_ptr;
    if ((units == "miles") || (units == "mi")) {
      directions_options.set_units(DirectionsOptions_Units_kMiles);
    } else {
      directions_options.set_units(DirectionsOptions_Units_kKilometers);
    }
  }

  auto lang_ptr = pt.get_optional<std::string>("language");
  if (lang_ptr) {
    directions_options.set_language(*lang_ptr);
  }

  auto narr_ptr = pt.get_optional<bool>("narrative");
  if (narr_ptr) {
    directions_options.set_narrative(*narr_ptr);
  }

  return directions_options;
}

//Get the time from the inputed date.
//date_time is in the format of 2015-05-06T08:00-05:00
std::string get_localized_time(const std::string& date_time,
                               const std::locale& locale) {
  if (date_time.find("T") == std::string::npos) return "";

  std::string time = date_time;
  try {
    // formatting for in and output
    std::locale in_locale(std::locale::classic(), new boost::local_time::local_time_input_facet("%Y-%m-%dT%H:%M%ZP"));
    std::stringstream in_stream; in_stream.imbue(in_locale);
    std::locale out_locale(locale, new boost::local_time::local_time_facet("%X"));
    std::stringstream out_stream; out_stream.imbue(out_locale);

    // This is our output date time.
    boost::local_time::local_date_time ldt(boost::local_time::not_a_date_time);

     // Read the timestamp into ldt.
    in_stream.str(date_time);
    in_stream >> ldt;

    //format the output
    out_stream << ldt;
    time = out_stream.str();

    //seconds is too granular so we try to remove
    if (time.find("PM") == std::string::npos
        && time.find("AM") == std::string::npos) {
      size_t found = time.find_last_of(":");
      if (found != std::string::npos)
        time = time.substr(0, found);
      else {
        found = time.find_last_of("00");
        if (found != std::string::npos)
          time = time.substr(0, found - 1);
      }
    } else {
      boost::replace_all(time, ":00 ", " ");
      if (time.substr(0, 1) == "0")
        time = time.substr(1, time.size());
    }
    boost::algorithm::trim(time);

    // Add the timezone abbreviation to the time.
    // TODO: when we support right to left languages, we need to handle the
    // placement of the timezone.
    std::locale tz_out_locale(locale, new boost::local_time::local_time_facet("%z"));
    std::stringstream tz_out_stream; tz_out_stream.imbue(tz_out_locale);

    //format the output
    tz_out_stream << ldt;
    if (!out_stream.str().empty())
      time += " " + tz_out_stream.str();

  } catch (std::exception&) { return ""; }

  return time;
}

//Get the date from the inputed date.
//date_time is in the format of 2015-05-06T08:00-05:00
std::string get_localized_date(const std::string& date_time,
                               const std::locale& locale) {
  if (date_time.find("T") == std::string::npos) return "";

  std::string date = date_time;
  try {

    // formatting for in and output
    std::locale in_locale(std::locale::classic(), new boost::local_time::local_time_input_facet("%Y-%m-%dT%H:%M%ZP"));
    std::stringstream in_stream; in_stream.imbue(in_locale);
    std::locale out_locale(locale, new boost::posix_time::time_facet("%x"));
    std::stringstream out_stream; out_stream.imbue(out_locale);

    //parse the input
    boost::posix_time::ptime pt;
    in_stream.str(date);
    in_stream >> pt;

    //format the output
    out_stream << pt;
    date = out_stream.str();
  } catch (std::exception& e) { return ""; }

  boost::algorithm::trim(date);
  return date;
}

const locales_singleton_t& get_locales() {
  //thread safe static initializer for singleton
  static locales_singleton_t locales(load_narrative_locals());
  return locales;
}

const std::unordered_map<std::string, std::string>& get_locales_json() {
  return locales_json;
}

}
}
