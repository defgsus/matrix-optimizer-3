/** @file

    inline code for UINT to string mapping for win32 WM_... codes
*/


#ifdef MO_OS_WIN

#if 0
#   define MO__DEBUG(arg__) { std::cerr << arg__ << std::endl; }
#else
#   define MO__DEBUG(unused__) { }
#endif

namespace Private {

    static std::map<UINT, std::string> event_name_;

    bool create_event_name_map_()
    {
        std::map<UINT, std::string>::const_iterator i = event_name_.begin();

        // little macro that puts the define 'expr'
        // into the map with "expr" as string
        // checks for clashes
        #define X(expr)                                              \
            i = event_name_.find(expr);                              \
            if (i==event_name_.end())                                \
                event_name_[expr] = #expr;                           \
            else                                                     \
                MO__DEBUG("WM_MESSAGE clash: " << i->second          \
                            << " <- " << #expr << " ("               \
                            << std::hex << expr << std::dec << ")");

        #include "wm_codes.def"

        #undef X

        // print list
        // for (auto i=event_name_.begin(); i!=event_name_.end(); ++i)
        //    std::cout << i->second << "\t"
        //              << std::hex << "0x" << i->first << std::endl;

        return true;
    }

    static bool dummy_ = create_event_name_map_();
}


/** returns the name of a WM_-event or "unknown(0x'event')" */
std::string get_event_name_(UINT event)
{
    std::stringstream s;

    std::map<UINT, std::string>::const_iterator i = Private::event_name_.find(event);
    if (i==Private::event_name_.end())
        s << "unknown(" << std::hex << event << std::dec << ")";
    else
        s << i->second;

    return s.str();
}

#undef MO__DEBUG

#endif
