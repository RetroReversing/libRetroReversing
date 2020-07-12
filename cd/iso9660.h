#ifndef ISO9660_H_
#define ISO9660_H_

// This is a generated file! Please edit source .ksy file and use kaitai-struct-compiler to rebuild

#include "kaitaistruct.h"

#include <stdint.h>
#include <vector>

#if KAITAI_STRUCT_VERSION < 7000L
#error "Incompatible Kaitai Struct C++/STL API: version 0.7 or later is required"
#endif

/**
 * ISO9660 is standard filesystem used on read-only optical discs
 * (mostly CD-ROM). The standard was based on earlier High Sierra
 * Format (HSF), proposed for CD-ROMs in 1985, and, after several
 * revisions, it was accepted as ISO9960:1998.
 * 
 * The format emphasizes portability (thus having pretty minimal
 * features and very conservative file names standards) and sequential
 * access (which favors disc devices with relatively slow rotation
 * speed).
 */

class iso9660_t : public kaitai::kstruct {

public:
    class vol_desc_primary_t;
    class vol_desc_boot_record_t;
    class datetime_t;
    class dir_entry_t;
    class vol_desc_t;
    class path_table_entry_le_t;
    class dir_entries_t;
    class u4bi_t;
    class u2bi_t;
    class path_table_le_t;
    class dec_datetime_t;
    class dir_entry_body_t;

    iso9660_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, iso9660_t* p__root = 0);

private:
    void _read();

public:
    ~iso9660_t();

    /**
     * \sa Source
     */

    class vol_desc_primary_t : public kaitai::kstruct {

    public:

        vol_desc_primary_t(kaitai::kstream* p__io, iso9660_t::vol_desc_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~vol_desc_primary_t();

    private:
        bool f_path_table;
        path_table_le_t* m_path_table;

    public:
        path_table_le_t* path_table();

    private:
        std::string m_unused1;
        std::string m_system_id;
        std::string m_volume_id;
        std::string m_unused2;
        u4bi_t* m_vol_space_size;
        std::string m_unused3;
        u2bi_t* m_vol_set_size;
        u2bi_t* m_vol_seq_num;
        u2bi_t* m_logical_block_size;
        u4bi_t* m_path_table_size;
        uint32_t m_lba_path_table_le;
        uint32_t m_lba_opt_path_table_le;
        uint32_t m_lba_path_table_be;
        uint32_t m_lba_opt_path_table_be;
        dir_entry_t* m_root_dir;
        std::string m_vol_set_id;
        std::string m_publisher_id;
        std::string m_data_preparer_id;
        std::string m_application_id;
        std::string m_copyright_file_id;
        std::string m_abstract_file_id;
        std::string m_bibliographic_file_id;
        dec_datetime_t* m_vol_create_datetime;
        dec_datetime_t* m_vol_mod_datetime;
        dec_datetime_t* m_vol_expire_datetime;
        dec_datetime_t* m_vol_effective_datetime;
        uint8_t m_file_structure_version;
        uint8_t m_unused4;
        std::string m_application_area;
        iso9660_t* m__root;
        iso9660_t::vol_desc_t* m__parent;
        std::string m__raw_root_dir;
        kaitai::kstream* m__io__raw_root_dir;
        std::string m__raw_path_table;
        kaitai::kstream* m__io__raw_path_table;

    public:
        std::string unused1() const { return m_unused1; }
        std::string system_id() const { return m_system_id; }
        std::string volume_id() const { return m_volume_id; }
        std::string unused2() const { return m_unused2; }
        u4bi_t* vol_space_size() const { return m_vol_space_size; }
        std::string unused3() const { return m_unused3; }
        u2bi_t* vol_set_size() const { return m_vol_set_size; }
        u2bi_t* vol_seq_num() const { return m_vol_seq_num; }
        u2bi_t* logical_block_size() const { return m_logical_block_size; }
        u4bi_t* path_table_size() const { return m_path_table_size; }
        uint32_t lba_path_table_le() const { return m_lba_path_table_le; }
        uint32_t lba_opt_path_table_le() const { return m_lba_opt_path_table_le; }
        uint32_t lba_path_table_be() const { return m_lba_path_table_be; }
        uint32_t lba_opt_path_table_be() const { return m_lba_opt_path_table_be; }
        dir_entry_t* root_dir() const { return m_root_dir; }
        std::string vol_set_id() const { return m_vol_set_id; }
        std::string publisher_id() const { return m_publisher_id; }
        std::string data_preparer_id() const { return m_data_preparer_id; }
        std::string application_id() const { return m_application_id; }
        std::string copyright_file_id() const { return m_copyright_file_id; }
        std::string abstract_file_id() const { return m_abstract_file_id; }
        std::string bibliographic_file_id() const { return m_bibliographic_file_id; }
        dec_datetime_t* vol_create_datetime() const { return m_vol_create_datetime; }
        dec_datetime_t* vol_mod_datetime() const { return m_vol_mod_datetime; }
        dec_datetime_t* vol_expire_datetime() const { return m_vol_expire_datetime; }
        dec_datetime_t* vol_effective_datetime() const { return m_vol_effective_datetime; }
        uint8_t file_structure_version() const { return m_file_structure_version; }
        uint8_t unused4() const { return m_unused4; }
        std::string application_area() const { return m_application_area; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::vol_desc_t* _parent() const { return m__parent; }
        std::string _raw_root_dir() const { return m__raw_root_dir; }
        kaitai::kstream* _io__raw_root_dir() const { return m__io__raw_root_dir; }
        std::string _raw_path_table() const { return m__raw_path_table; }
        kaitai::kstream* _io__raw_path_table() const { return m__io__raw_path_table; }
    };

    class vol_desc_boot_record_t : public kaitai::kstruct {

    public:

        vol_desc_boot_record_t(kaitai::kstream* p__io, iso9660_t::vol_desc_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~vol_desc_boot_record_t();

    private:
        std::string m_boot_system_id;
        std::string m_boot_id;
        iso9660_t* m__root;
        iso9660_t::vol_desc_t* m__parent;

    public:
        std::string boot_system_id() const { return m_boot_system_id; }
        std::string boot_id() const { return m_boot_id; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::vol_desc_t* _parent() const { return m__parent; }
    };

    class datetime_t : public kaitai::kstruct {

    public:

        datetime_t(kaitai::kstream* p__io, iso9660_t::dir_entry_body_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~datetime_t();

    private:
        uint8_t m_year;
        uint8_t m_month;
        uint8_t m_day;
        uint8_t m_hour;
        uint8_t m_minute;
        uint8_t m_sec;
        uint8_t m_timezone;
        iso9660_t* m__root;
        iso9660_t::dir_entry_body_t* m__parent;

    public:
        uint8_t year() const { return m_year; }
        uint8_t month() const { return m_month; }
        uint8_t day() const { return m_day; }
        uint8_t hour() const { return m_hour; }
        uint8_t minute() const { return m_minute; }
        uint8_t sec() const { return m_sec; }
        uint8_t timezone() const { return m_timezone; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::dir_entry_body_t* _parent() const { return m__parent; }
    };

    class dir_entry_t : public kaitai::kstruct {

    public:

        dir_entry_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~dir_entry_t();

    private:
        uint8_t m_len;
        dir_entry_body_t* m_body;
        bool n_body;

    public:
        bool _is_null_body() { body(); return n_body; };

    private:
        iso9660_t* m__root;
        kaitai::kstruct* m__parent;
        std::string m__raw_body;
        kaitai::kstream* m__io__raw_body;

    public:
        uint8_t len() const { return m_len; }
        dir_entry_body_t* body() const { return m_body; }
        iso9660_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
        std::string _raw_body() const { return m__raw_body; }
        kaitai::kstream* _io__raw_body() const { return m__io__raw_body; }
    };

    class vol_desc_t : public kaitai::kstruct {

    public:

        vol_desc_t(kaitai::kstream* p__io, iso9660_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~vol_desc_t();

    private:
        uint8_t m_type;
        std::string m_magic;
        uint8_t m_version;
        vol_desc_boot_record_t* m_vol_desc_boot_record;
        bool n_vol_desc_boot_record;

    public:
        bool _is_null_vol_desc_boot_record() { vol_desc_boot_record(); return n_vol_desc_boot_record; };

    private:
        vol_desc_primary_t* m_vol_desc_primary;
        bool n_vol_desc_primary;

    public:
        bool _is_null_vol_desc_primary() { vol_desc_primary(); return n_vol_desc_primary; };

    private:
        iso9660_t* m__root;
        iso9660_t* m__parent;

    public:
        uint8_t type() const { return m_type; }
        std::string magic() const { return m_magic; }
        uint8_t version() const { return m_version; }
        vol_desc_boot_record_t* vol_desc_boot_record() const { return m_vol_desc_boot_record; }
        vol_desc_primary_t* vol_desc_primary() const { return m_vol_desc_primary; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t* _parent() const { return m__parent; }
    };

    class path_table_entry_le_t : public kaitai::kstruct {

    public:

        path_table_entry_le_t(kaitai::kstream* p__io, iso9660_t::path_table_le_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~path_table_entry_le_t();

    private:
        uint8_t m_len_dir_name;
        uint8_t m_len_ext_attr_rec;
        uint32_t m_lba_extent;
        uint16_t m_parent_dir_idx;
        std::string m_dir_name;
        uint8_t m_padding;
        bool n_padding;

    public:
        bool _is_null_padding() { padding(); return n_padding; };

    private:
        iso9660_t* m__root;
        iso9660_t::path_table_le_t* m__parent;

    public:
        uint8_t len_dir_name() const { return m_len_dir_name; }
        uint8_t len_ext_attr_rec() const { return m_len_ext_attr_rec; }
        uint32_t lba_extent() const { return m_lba_extent; }
        uint16_t parent_dir_idx() const { return m_parent_dir_idx; }
        std::string dir_name() const { return m_dir_name; }
        uint8_t padding() const { return m_padding; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::path_table_le_t* _parent() const { return m__parent; }
    };

    class dir_entries_t : public kaitai::kstruct {

    public:

        dir_entries_t(kaitai::kstream* p__io, iso9660_t::dir_entry_body_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~dir_entries_t();

    private:
        std::vector<dir_entry_t*>* m_entries;
        iso9660_t* m__root;
        iso9660_t::dir_entry_body_t* m__parent;

    public:
        std::vector<dir_entry_t*>* entries() const { return m_entries; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::dir_entry_body_t* _parent() const { return m__parent; }
    };

    class u4bi_t : public kaitai::kstruct {

    public:

        u4bi_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~u4bi_t();

    private:
        uint32_t m_le;
        uint32_t m_be;
        iso9660_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint32_t le() const { return m_le; }
        uint32_t be() const { return m_be; }
        iso9660_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    class u2bi_t : public kaitai::kstruct {

    public:

        u2bi_t(kaitai::kstream* p__io, kaitai::kstruct* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~u2bi_t();

    private:
        uint16_t m_le;
        uint16_t m_be;
        iso9660_t* m__root;
        kaitai::kstruct* m__parent;

    public:
        uint16_t le() const { return m_le; }
        uint16_t be() const { return m_be; }
        iso9660_t* _root() const { return m__root; }
        kaitai::kstruct* _parent() const { return m__parent; }
    };

    /**
     * \sa Source
     */

    class path_table_le_t : public kaitai::kstruct {

    public:

        path_table_le_t(kaitai::kstream* p__io, iso9660_t::vol_desc_primary_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~path_table_le_t();

    private:
        std::vector<path_table_entry_le_t*>* m_entries;
        iso9660_t* m__root;
        iso9660_t::vol_desc_primary_t* m__parent;

    public:
        std::vector<path_table_entry_le_t*>* entries() const { return m_entries; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::vol_desc_primary_t* _parent() const { return m__parent; }
    };

    /**
     * \sa Source
     */

    class dec_datetime_t : public kaitai::kstruct {

    public:

        dec_datetime_t(kaitai::kstream* p__io, iso9660_t::vol_desc_primary_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~dec_datetime_t();

    private:
        std::string m_year;
        std::string m_month;
        std::string m_day;
        std::string m_hour;
        std::string m_minute;
        std::string m_sec;
        std::string m_sec_hundreds;
        uint8_t m_timezone;
        iso9660_t* m__root;
        iso9660_t::vol_desc_primary_t* m__parent;

    public:
        std::string year() const { return m_year; }
        std::string month() const { return m_month; }
        std::string day() const { return m_day; }
        std::string hour() const { return m_hour; }
        std::string minute() const { return m_minute; }
        std::string sec() const { return m_sec; }
        std::string sec_hundreds() const { return m_sec_hundreds; }
        uint8_t timezone() const { return m_timezone; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::vol_desc_primary_t* _parent() const { return m__parent; }
    };

    class dir_entry_body_t : public kaitai::kstruct {

    public:

        dir_entry_body_t(kaitai::kstream* p__io, iso9660_t::dir_entry_t* p__parent = 0, iso9660_t* p__root = 0);

    private:
        void _read();

    public:
        ~dir_entry_body_t();

    private:
        bool f_extent_as_dir;
        dir_entries_t* m_extent_as_dir;
        bool n_extent_as_dir;

    public:
        bool _is_null_extent_as_dir() { extent_as_dir(); return n_extent_as_dir; };

    private:

    public:
        dir_entries_t* extent_as_dir();

    private:
        bool f_extent_as_file;
        std::string m_extent_as_file;
        bool n_extent_as_file;

    public:
        bool _is_null_extent_as_file() { extent_as_file(); return n_extent_as_file; };

    private:

    public:
        std::string extent_as_file();

    private:
        uint8_t m_len_ext_attr_rec;
        u4bi_t* m_lba_extent;
        u4bi_t* m_size_extent;
        datetime_t* m_datetime;
        uint8_t m_file_flags;
        uint8_t m_file_unit_size;
        uint8_t m_interleave_gap_size;
        u2bi_t* m_vol_seq_num;
        uint8_t m_len_file_name;
        std::string m_file_name;
        uint8_t m_padding;
        bool n_padding;

    public:
        bool _is_null_padding() { padding(); return n_padding; };

    private:
        std::string m_rest;
        iso9660_t* m__root;
        iso9660_t::dir_entry_t* m__parent;
        std::string m__raw_extent_as_dir;
        kaitai::kstream* m__io__raw_extent_as_dir;

    public:
        uint8_t len_ext_attr_rec() const { return m_len_ext_attr_rec; }
        u4bi_t* lba_extent() const { return m_lba_extent; }
        u4bi_t* size_extent() const { return m_size_extent; }
        datetime_t* datetime() const { return m_datetime; }
        uint8_t file_flags() const { return m_file_flags; }
        uint8_t file_unit_size() const { return m_file_unit_size; }
        uint8_t interleave_gap_size() const { return m_interleave_gap_size; }
        u2bi_t* vol_seq_num() const { return m_vol_seq_num; }
        uint8_t len_file_name() const { return m_len_file_name; }
        std::string file_name() const { return m_file_name; }
        uint8_t padding() const { return m_padding; }
        std::string rest() const { return m_rest; }
        iso9660_t* _root() const { return m__root; }
        iso9660_t::dir_entry_t* _parent() const { return m__parent; }
        std::string _raw_extent_as_dir() const { return m__raw_extent_as_dir; }
        kaitai::kstream* _io__raw_extent_as_dir() const { return m__io__raw_extent_as_dir; }
    };

private:
    bool f_sector_size;
    int32_t m_sector_size;

public:
    int32_t sector_size();

private:
    bool f_primary_vol_desc;
    vol_desc_t* m_primary_vol_desc;

public:
    vol_desc_t* primary_vol_desc();

private:
    iso9660_t* m__root;
    kaitai::kstruct* m__parent;

public:
    iso9660_t* _root() const { return m__root; }
    kaitai::kstruct* _parent() const { return m__parent; }
};

#endif  // ISO9660_H_
