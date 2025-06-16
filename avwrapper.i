%module avwrapper

%{
#include "avwrapper.h"
#include <libavcodec/avcodec.h>
%}

// read-only Felder mit %immutable
%immutable AVCodecParameters::bit_rate;
%immutable AVCodecParameters::width;
%immutable AVCodecParameters::height;
%immutable AVCodecParameters::codec_id;
%immutable AVCodecParameters::codec_type;

%include "typemaps.i"
%include "cpointer.i"
%include "stdint.i"
%include "avwrapper.h"

// Direktes Einbinden von AVCodecParameters-Strukturdefinition (für Feldzugriff)
%include <libavcodec/codec_par.h>

// Hilfsfunktion für codec name
%inline %{
    const char* get_codec_name(enum AVCodecID id) {
        return avcodec_get_name(id);
    }
%}
