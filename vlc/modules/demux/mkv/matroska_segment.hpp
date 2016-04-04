/*****************************************************************************
 * matroska_segment.hpp : matroska demuxer
 *****************************************************************************
 * Copyright (C) 2003-2004 VLC authors and VideoLAN
 * $Id$
 *
 * Authors: Laurent Aimar <fenrir@via.ecp.fr>
 *          Steve Lhomme <steve.lhomme@free.fr>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2.1 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston MA 02110-1301, USA.
 *****************************************************************************/

#ifndef VLC_MKV_MATROSKA_SEGMENT_HPP_
#define VLC_MKV_MATROSKA_SEGMENT_HPP_

#include "mkv.hpp"
#include <vector>
#include <string>

class EbmlParser;

class chapter_edition_c;
class chapter_translation_c;
class chapter_item_c;

struct mkv_track_t;
struct mkv_index_t;

typedef enum
{
    WHOLE_SEGMENT,
    TRACK_UID,
    EDITION_UID,
    CHAPTER_UID,
    ATTACHMENT_UID
} tag_target_type;

class SimpleTag
{
public:
    typedef std::vector<SimpleTag> sub_tags_t;
    std::string tag_name;
    std::string lang;
    std::string value;
    sub_tags_t sub_tags;
};

class Tag
{
public:
    typedef std::vector<SimpleTag> simple_tags_t;
    Tag():i_tag_type(WHOLE_SEGMENT),i_target_type(50),i_uid(0){}
    tag_target_type i_tag_type;
    uint64_t        i_target_type;
    uint64_t        i_uid;
    simple_tags_t   simple_tags;
};

class matroska_segment_c
{
public:
    typedef std::vector<mkv_index_t> indexes_t;
    typedef std::vector<Tag>            tags_t;

    matroska_segment_c( demux_sys_t & demuxer, EbmlStream & estream );
    virtual ~matroska_segment_c();

    KaxSegment              *segment;
    EbmlStream              & es;

    /* time scale */
    uint64_t                i_timescale;

    /* duration of the segment */
    mtime_t                 i_duration;
    mtime_t                 i_mk_start_time;

    /* all tracks */
    std::vector<mkv_track_t*> tracks;

    /* from seekhead */
    int                     i_seekhead_count;
    int64_t                 i_seekhead_position;
    int64_t                 i_cues_position;
    int64_t                 i_tracks_position;
    int64_t                 i_info_position;
    int64_t                 i_chapters_position;
    int64_t                 i_attachments_position;

    KaxCluster              *cluster;
    uint64                  i_block_pos;
    uint64                  i_cluster_pos;
    int64_t                 i_start_pos;
    KaxSegmentUID           *p_segment_uid;
    KaxPrevUID              *p_prev_segment_uid;
    KaxNextUID              *p_next_segment_uid;

    bool                    b_cues;
    indexes_t               indexes;

    /* info */
    char                    *psz_muxing_application;
    char                    *psz_writing_application;
    char                    *psz_segment_filename;
    char                    *psz_title;
    char                    *psz_date_utc;

    /* !!!!! GCC 3.3 bug on Darwin !!!!! */
    /* when you remove this variable the compiler issues an atomicity error */
    /* this variable only works when using std::vector<chapter_edition_c> */
    std::vector<chapter_edition_c*> stored_editions;
    std::vector<chapter_edition_c*>::size_type i_default_edition;

    std::vector<chapter_translation_c*> translations;
    std::vector<KaxSegmentFamily*>  families;
    tags_t                          tags;

    demux_sys_t                    & sys;
    EbmlParser                     *ep;
    bool                           b_preloaded;
    bool                           b_ref_external_segments;

    bool Preload();
    bool PreloadFamily( const matroska_segment_c & segment );
    void InformationCreate();
    void Seek( mtime_t i_mk_date, mtime_t i_mk_time_offset, int64_t i_global_position );
    int BlockGet( KaxBlock * &, KaxSimpleBlock * &, bool *, bool *, int64_t *);

    int BlockFindTrackIndex( size_t *pi_track,
                             const KaxBlock *, const KaxSimpleBlock * );

    bool Select( mtime_t i_mk_start_time );
    void UnSelect();

    size_t        index_idx () const { return indexes.size () - 1; }
    mkv_index_t&      index ()       { return *(indexes.rbegin()); }
    mkv_index_t& prev_index ()       { return *(indexes.rbegin()+1); }

    indexes_t::iterator indexes_begin () { return indexes.begin(); }
    indexes_t::iterator indexes_end   () { return indexes.end() - (indexes.size() ? 1 : 0); }

    static bool CompareSegmentUIDs( const matroska_segment_c * item_a, const matroska_segment_c * item_b );

    bool SameFamily( const matroska_segment_c & of_segment ) const;

private:
    void LoadCues( KaxCues *cues );
    void LoadTags( KaxTags *tags );
    bool LoadSeekHeadItem( const EbmlCallbacks & ClassInfos, int64_t i_element_position );
    void ParseInfo( KaxInfo *info );
    void ParseAttachments( KaxAttachments *attachments );
    void ParseChapters( KaxChapters *chapters );
    void ParseSeekHead( KaxSeekHead *seekhead );
    void ParseTracks( KaxTracks *tracks );
    void ParseChapterAtom( int i_level, KaxChapterAtom *ca, chapter_item_c & chapters );
    void ParseTrackEntry( KaxTrackEntry *m );
    void ParseCluster( KaxCluster *cluster, bool b_update_start_time = true, ScopeMode read_fully = SCOPE_ALL_DATA );
    bool ParseSimpleTags( SimpleTag* out, KaxTagSimple *tag, int level = 50 );
    void IndexAppendCluster( KaxCluster *cluster );
    int32_t TrackInit( mkv_track_t * p_tk );
    void ComputeTrackPriority();
    void EnsureDuration();
};


#endif