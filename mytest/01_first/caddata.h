
#ifndef __X_TEST_EXAMPLE_H
#define __X_TEST_EXAMPLE_H

#define XTOSTRUCT_SUPPORT_CHAR_ARRAY
#include <x2struct.hpp>
//#include <xtypes.h>

enum Mode {
    MASTER = 1,
    SLAVE = 2,
};

struct condition {
    int         id;
    std::string url;
    XTOSTRUCT(M(url));
};

struct sub {
    int    a;
    std::string b;
    XTOSTRUCT(M(a), O(b));
};

struct SharePtr {
    int a;
    XTOSTRUCT(O(a));
};

struct xstruct {
    int    id;
    x2struct::XDate  start;
    int    tint;
    std::string tstring;
    char   chArray[16];

    SharePtr sp;
    std::map<std::string, int> umap;
    std::vector<int> vint;
    std::list<int> lint;
    std::vector<std::string> vstring;
    std::vector<int64_t> vlong;
    std::vector<sub> vsub;
    std::vector<std::vector<int> > vvint;
    std::vector<std::vector<std::string> > vvstring;
    std::vector<std::vector<sub> > vvsub;
    std::map<int, sub> tmap;
    Mode md;

    XTOSTRUCT(A(id,"config:id _id,m"), O(start, tint, tstring, chArray, sp, umap, vint, lint, vstring, vlong, vsub, vvint, vvstring, vvsub, tmap, md));
};

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
    XTOSTRUCT(O(user_defined_model_pathname,
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
    XTOSTRUCT(O(abilities,
                caps_state,
                ui_owner,
                size_max_x,
                size_max_y,
                description));
};

struct Calibration {
    double _x;
    double _y;
    XTOSTRUCT(O(_x,
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
    
    XTOSTRUCT(O(caps_type,
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
    XTOSTRUCT(O(media));
};

struct Udm {
    struct Calibration calibration;
    struct Media media;
    XTOSTRUCT(O(calibration,
                media));
};

struct PublistToWeb
{
    struct Meta meta;
    struct MediaWrapper mod;
    struct MediaWrapper del;
    struct Udm udm;
    struct MediaWrapper hidden;
    
    XTOSTRUCT(O(meta,
                mod,
                del,
                udm,
                hidden));
};

#endif
