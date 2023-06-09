#ifndef EMANERELAY_LOGGER_H_
#define EMANERELAY_LOGGER_H_

namespace emane_relay {

    class Logger
    {
        public:
            Logger(int level);
            ~Logger();

        // private:
            int level_;
    };

    extern Logger * LOG;
}

#endif // EMANERELAY_LOGGER_H_