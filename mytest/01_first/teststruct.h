#pragma once

#include "xpack/json.h"

struct Meta {
    std::string user_defined_model_pathname;
    std::string user_defined_model_basename;
    std::string driver_pathname;
    std::string driver_version;
    std::string driver_tag_line;
    int toolkit_version;
    int driver_type;
    std::string canonical_family_name;
    bool show_custom_first;
    bool truetype_as_text;
    std::string canonical_model_name;
    std::string localized_family_name;
    std::string localized_model_name;
    bool file_only;
    std::string model_abilities;
    std::string udm_description;
    std::string win_device_name;
    std::string win_driver_name;
    std::string short_net_name;
    std::string friendly_net_name;
    int dm_driver_version;
    bool default_system_cfg;
    std::string platform;
    std::string locale;
    XPACK(O(user_defined_model_pathname,
                user_defined_model_basename,
                driver_pathname,
                driver_version,
                driver_tag_line,
                toolkit_version,
                driver_type,
                canonical_family_name,
                show_custom_first,
                truetype_as_text,
                canonical_model_name,
                localized_family_name,
                localized_model_name,
                file_only,
                model_abilities,
                udm_description,
                win_device_name,
                win_driver_name,
                short_net_name,
                friendly_net_name,
                dm_driver_version,
                default_system_cfg,
                platform,
                locale));
};

struct Media {
    std::string abilities;
    std::string caps_state;
    std::string ui_owner;
    double size_max_x;
    double size_max_y;
    std::vector<struct DescriptionElement> description;
    XPACK(O(abilities,
                caps_state,
                ui_owner,
                size_max_x,
                size_max_y,
                description));
};

struct Calibration {
    double _x;
    double _y;
    XPACK(O(_x,
                _y));
};

struct DescriptionElement {
    int caps_type;
    std::string name;
    double media_bounds_urx;
    double media_bounds_ury;
    double printable_bounds_llx;
    double printable_bounds_lly;
    double printable_bounds_urx;
    double printable_bounds_ury;
    double printable_area;
    double dimensional;
    
    XPACK(O(caps_type,
                name,
                media_bounds_urx,
                media_bounds_ury,
                printable_bounds_llx,
                printable_bounds_lly,
                printable_bounds_urx,
                printable_bounds_ury,
                printable_area,
                dimensional));
};

struct MediaWrapper {
    struct Media media;
    XPACK(O(media));
};

struct Udm {
    struct Calibration calibration;
    struct Media media;
    XPACK(O(calibration,
                media));
};

struct PublistToWeb
{
    struct Meta meta;
    struct MediaWrapper mod;
    struct MediaWrapper del;
    struct Udm udm;
    struct MediaWrapper hidden;
    
    XPACK(O(meta,
                mod,
                del,
                udm,
                hidden));
};

