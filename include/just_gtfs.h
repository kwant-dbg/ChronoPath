#pragma once

#include <algorithm>
#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <iterator>
#include <limits>
#include <map>
#include <memory>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace gtfs
{
using Id = std::string;
constexpr char const * kCsvDelimiter = ",";
constexpr char const * kCsvLineDelimiter = "\n";

inline namespace
{
  std::vector<std::string_view> Split(std::string_view s, std::string_view delimiter)
  {
    std::vector<std::string_view> result;
    size_t last = 0;
    size_t next = 0;
    while ((next = s.find(delimiter, last)) != std::string_view::npos)
    {
      result.emplace_back(s.substr(last, next - last));
      last = next + delimiter.length();
    }
    if (s.length() - last > 0)
      result.emplace_back(s.substr(last));
    return result;
  }

  std::string_view Trim(std::string_view s)
  {
    s.remove_prefix(std::min(s.find_first_not_of(" \t\r\v\n"), s.size()));
    s.remove_suffix(std::min(s.size() - s.find_last_not_of(" \t\r\v\n") - 1, s.size()));
    return s;
  }
}  // namespace

// A helper for reading GTFS csv files.
class GtfsReader
{
public:
  // @param dir is a path to a directory with a GTFS feed.
  // @param file_name is a name of a GTFS file. For example "stops.txt"
  GtfsReader(std::string const & dir, std::string const & file_name)
  {
    m_path = dir + file_name;
    m_file.open(m_path);
    if (!m_file)
      return;
    m_good = true;

    // Reading the header.
    std::string line;
    if (!std::getline(m_file, line))
    {
      m_good = false;
      return;
    }

    auto const header_fields = Split(line, kCsvDelimiter);
    for (size_t i = 0; i < header_fields.size(); ++i)
      m_field_to_pos[std::string(Trim(header_fields[i]))] = i;
  }

  bool IsGood() const { return m_good; }

  // Reads a GTFS file line by line.
  // @param on_line is a callback which is called on each line except for the header.
  template <typename OnLine>
  void Read(OnLine && on_line)
  {
    if (!IsGood())
      return;

    std::string line;
    while (std::getline(m_file, line))
    {
      if (line.empty() || line.back() != '\r' && line.back() != '\n')
      {
        auto const last_pos = m_file.tellg();
        m_file.seekg(0, m_file.end);
        if (last_pos != m_file.tellg())
          line.push_back('\n');
        m_file.seekg(last_pos);
      }

      on_line(line);
    }
  }

  size_t GetFieldsCount() const { return m_field_to_pos.size(); }
  bool HasField(std::string const & field_name) const
  {
    return m_field_to_pos.find(field_name) != m_field_to_pos.cend();
  }
  size_t GetFieldPos(std::string const & field_name) const
  {
    auto const it = m_field_to_pos.find(field_name);
    if (it == m_field_to_pos.cend())
    {
      std::stringstream msg;
      msg << "The file " << m_path << " does not have field " << field_name;
      throw std::runtime_error(msg.str());
    }
    return it->second;
  }

private:
  bool m_good = false;
  std::string m_path;
  std::ifstream m_file;
  std::map<std::string, size_t> m_field_to_pos;
};

// Represents a row from any GTFS file.
class GtfsRecord
{
public:
  GtfsRecord(GtfsReader const * reader, std::string_view s)
    : m_reader(reader)
    , m_fields(Split(s, kCsvDelimiter))
  {
    // Removing trailing \r from the last field if it exists.
    if (!m_fields.empty() && !m_fields.back().empty() && m_fields.back().back() == '\r')
      m_fields.back().remove_suffix(1);
  }

  // @return an item from a row by a field name.
  template <typename T>
  T Get(std::string const & field_name) const
  {
    auto const & s = GetString(field_name);
    T res;
    std::stringstream ss;
    ss << s;
    ss >> res;
    return res;
  }

  // @return an item from a row by a field name. The result may be empty if the field is optional.
  template <typename T>
  std::optional<T> GetOptional(std::string const & field_name) const
  {
    if (!m_reader->HasField(field_name))
      return {};

    auto const & s = GetString(field_name);
    if (s.empty())
      return {};

    return Get<T>(field_name);
  }

private:
  std::string_view GetString(std::string const & field_name) const
  {
    auto const pos = m_reader->GetFieldPos(field_name);
    if (pos >= m_fields.size())
    {
      std::stringstream msg;
      msg << "The row " << DebugPrint() << " has not field " << field_name;
      throw std::runtime_error(msg.str());
    }
    return Trim(m_fields[pos]);
  }

  std::string DebugPrint() const
  {
    std::stringstream ss;
    for (size_t i = 0; i < m_fields.size(); ++i)
    {
      ss << m_fields[i];
      if (i + 1 != m_fields.size())
        ss << ", ";
    }
    return ss.str();
  }

  GtfsReader const * m_reader = nullptr;
  std::vector<std::string_view> m_fields;
};

template <>
inline std::string GtfsRecord::Get<std::string>(std::string const & field_name) const
{
  return std::string(GetString(field_name));
}

// See https://developers.google.com/transit/gtfs/reference/#routestxt
struct Route
{
  Id route_id;
  std::optional<Id> agency_id;
  std::optional<std::string> route_short_name;
  std::optional<std::string> route_long_name;
  std::optional<std::string> route_desc;
  // See https://developers.google.com/transit/gtfs/reference/extended-route-types
  int route_type;
  std::optional<std::string> route_url;
  std::optional<std::string> route_color;
  std::optional<std::string> route_text_color;
  std::optional<int> route_sort_order;
};

// See https://developers.google.com/transit/gtfs/reference/#stopstxt
enum class LocationType
{
  StopPoint = 0,
  Station = 1,
  EntranceExit = 2,
  GenericNode = 3,
  BoardingArea = 4,
  NotDefined = 5
};

enum class WheelchairBoarding
{
  NoInformation = 0,
  SomeRiders = 1,
  NoRiders = 2,
  NotDefined = 3
};

struct Stop
{
  Id stop_id;
  std::optional<std::string> stop_code;
  std::optional<std::string> stop_name;
  std::optional<std::string> stop_desc;
  double stop_lat;
  double stop_lon;
  std::optional<Id> zone_id;
  std::optional<std::string> stop_url;
  LocationType location_type = LocationType::NotDefined;
  std::optional<Id> parent_station;
  std::optional<std::string> stop_timezone;
  WheelchairBoarding wheelchair_boarding = WheelchairBoarding::NotDefined;
  std::optional<Id> level_id;
  std::optional<std::string> platform_code;
};

// See https://developers.google.com/transit/gtfs/reference/#tripstxt
enum class WheelchairAccessible
{
  NoInformation = 0,
  AtLeastOneRider = 1,
  NoRiders = 2,
  NotDefined = 3
};

enum class BikesAllowed
{
  NoInformation = 0,
  AtLeastOneBike = 1,
  NoBikes = 2,
  NotDefined = 3
};

struct Trip
{
  Id route_id;
  Id service_id;
  Id trip_id;
  std::optional<std::string> trip_headsign;
  std::optional<std::string> trip_short_name;
  std::optional<int> direction_id;
  std::optional<Id> block_id;
  std::optional<Id> shape_id;
  WheelchairAccessible wheelchair_accessible = WheelchairAccessible::NotDefined;
  BikesAllowed bikes_allowed = BikesAllowed::NotDefined;
};

// See https://developers.google.com/transit/gtfs/reference/#stop_timestxt
enum class Timepoint
{
  Approximate = 0,
  Exact = 1,
  NotDefined = 2
};

enum class PickupType
{
  RegularlyScheduled = 0,
  None = 1,
  MustPhone = 2,
  MustCoordinateWithDriver = 3,
  NotDefined = 4
};

enum class DropoffType
{
  RegularlyScheduled = 0,
  None = 1,
  MustPhone = 2,
  MustCoordinateWithDriver = 3,
  NotDefined = 4
};

struct StopTime
{
  Id trip_id;
  std::string arrival_time;
  std::string departure_time;
  Id stop_id;
  int stop_sequence;
  std::optional<std::string> stop_headsign;
  PickupType pickup_type = PickupType::NotDefined;
  DropoffType dropoff_type = DropoffType::NotDefined;
  std::optional<double> shape_dist_traveled;
  Timepoint timepoint = Timepoint::NotDefined;
};

enum class ResultCode
{
  OK,
  // The feed path is wrong.
  ERROR_INCORRECT_FEED_PATH,
  // Some of files are absent.
  ERROR_NO_REQUIRED_FILES,
  // Incorrect format of some of files.
  ERROR_INCORRECT_FORMAT
};

using Routes = std::vector<Route>;
using Stops = std::vector<Stop>;
using Trips = std::vector<Trip>;
using StopTimes = std::vector<StopTime>;

// Class for working with GTFS feeds.
class Feed
{
public:
  Feed(std::string const & path) : m_path(path) {}

  ResultCode read_feed()
  {
    if (m_path.empty())
      return ResultCode::ERROR_INCORRECT_FEED_PATH;
    if (m_path.back() != '/')
      m_path.push_back('/');

    auto res = read_stops();
    if (res != ResultCode::OK)
      return res;

    res = read_routes();
    if (res != ResultCode::OK)
      return res;

    res = read_trips();
    if (res != ResultCode::OK)
      return res;

    res = read_stop_times();
    if (res != ResultCode::OK)
      return res;

    return ResultCode::OK;
  }

  Routes const & get_routes() const { return m_routes; }
  Stops const & get_stops() const { return m_stops; }
  Trips const & get_trips() const { return m_trips; }
  StopTimes const & get_stop_times() const { return m_stop_times; }

private:
  ResultCode read_routes() { return read(&Feed::read_route, "routes.txt", &m_routes); }
  ResultCode read_stops() { return read(&Feed::read_stop, "stops.txt", &m_stops); }
  ResultCode read_trips() { return read(&Feed::read_trip, "trips.txt", &m_trips); }
  ResultCode read_stop_times()
  {
    return read(&Feed::read_stop_time, "stop_times.txt", &m_stop_times);
  }

  void read_route(GtfsRecord const & record)
  {
    Route r;
    r.route_id = record.Get<std::string>("route_id");
    r.agency_id = record.GetOptional<std::string>("agency_id");
    r.route_short_name = record.GetOptional<std::string>("route_short_name");
    r.route_long_name = record.GetOptional<std::string>("route_long_name");
    r.route_desc = record.GetOptional<std::string>("route_desc");
    r.route_type = record.Get<int>("route_type");
    r.route_url = record.GetOptional<std::string>("route_url");
    r.route_color = record.GetOptional<std::string>("route_color");
    r.route_text_color = record.GetOptional<std::string>("route_text_color");
    r.route_sort_order = record.GetOptional<int>("route_sort_order");
    m_routes.emplace_back(std::move(r));
  }

  void read_stop(GtfsRecord const & record)
  {
    Stop s;
    s.stop_id = record.Get<std::string>("stop_id");
    s.stop_code = record.GetOptional<std::string>("stop_code");
    s.stop_name = record.GetOptional<std::string>("stop_name");
    s.stop_desc = record.GetOptional<std::string>("stop_desc");
    s.stop_lat = record.Get<double>("stop_lat");
    s.stop_lon = record.Get<double>("stop_lon");
    s.zone_id = record.GetOptional<std::string>("zone_id");
    s.stop_url = record.GetOptional<std::string>("stop_url");
    auto const loc_type = record.GetOptional<int>("location_type");
    if (loc_type)
      s.location_type = static_cast<LocationType>(*loc_type);
    s.parent_station = record.GetOptional<std::string>("parent_station");
    s.stop_timezone = record.GetOptional<std::string>("stop_timezone");
    auto const wh_boarding = record.GetOptional<int>("wheelchair_boarding");
    if (wh_boarding)
      s.wheelchair_boarding = static_cast<WheelchairBoarding>(*wh_boarding);
    s.level_id = record.GetOptional<std::string>("level_id");
    s.platform_code = record.GetOptional<std::string>("platform_code");
    m_stops.emplace_back(std::move(s));
  }

  void read_trip(GtfsRecord const & record)
  {
    Trip t;
    t.route_id = record.Get<std::string>("route_id");
    t.service_id = record.Get<std::string>("service_id");
    t.trip_id = record.Get<std::string>("trip_id");
    t.trip_headsign = record.GetOptional<std::string>("trip_headsign");
    t.trip_short_name = record.GetOptional<std::string>("trip_short_name");
    t.direction_id = record.GetOptional<int>("direction_id");
    t.block_id = record.GetOptional<std::string>("block_id");
    t.shape_id = record.GetOptional<std::string>("shape_id");
    auto const wh_accessible = record.GetOptional<int>("wheelchair_accessible");
    if (wh_accessible)
      t.wheelchair_accessible = static_cast<WheelchairAccessible>(*wh_accessible);
    auto const bikes_allowed = record.GetOptional<int>("bikes_allowed");
    if (bikes_allowed)
      t.bikes_allowed = static_cast<BikesAllowed>(*bikes_allowed);
    m_trips.emplace_back(std::move(t));
  }

  void read_stop_time(GtfsRecord const & record)
  {
    StopTime st;
    st.trip_id = record.Get<std::string>("trip_id");
    st.arrival_time = record.Get<std::string>("arrival_time");
    st.departure_time = record.Get<std::string>("departure_time");
    st.stop_id = record.Get<std::string>("stop_id");
    st.stop_sequence = record.Get<int>("stop_sequence");
    st.stop_headsign = record.GetOptional<std::string>("stop_headsign");
    auto const pickup_type = record.GetOptional<int>("pickup_type");
    if (pickup_type)
      st.pickup_type = static_cast<PickupType>(*pickup_type);
    auto const dropoff_type = record.GetOptional<int>("dropoff_type");
    if (dropoff_type)
      st.dropoff_type = static_cast<DropoffType>(*dropoff_type);
    st.shape_dist_traveled = record.GetOptional<double>("shape_dist_traveled");
    auto const timepoint = record.GetOptional<int>("timepoint");
    if (timepoint)
      st.timepoint = static_cast<Timepoint>(*timepoint);
    m_stop_times.emplace_back(std::move(st));
  }

  template <typename Reader, typename Container>
  ResultCode read(Reader && reader, std::string const & file_name, Container * cont)
  {
    GtfsReader gtfs_reader(m_path, file_name);
    if (!gtfs_reader.IsGood())
      return ResultCode::ERROR_INCORRECT_FORMAT;

    gtfs_reader.Read([&](std::string_view s)
    {
      GtfsRecord const record(&gtfs_reader, s);
      (this->*reader)(record);
    });
    return ResultCode::OK;
  }

  std::string m_path;
  Routes m_routes;
  Stops m_stops;
  Trips m_trips;
  StopTimes m_stop_times;
};
}  // namespace gtfs
