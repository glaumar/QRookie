#pragma once
#ifndef QROOKIE_RCLONE_RESULT
#define QROOKIE_RCLONE_RESULT

#include <QString>

#include "librclone_wrapper.h"

class RcloneResult {
   public:
    RcloneResult(RcloneRPCResult result)
        : output_(result.Output), status_(result.Status) {}
    QString output() const { return output_; }
    int status() const { return status_; }
    bool isSuccessful() const { return status_ == 200; }

   private:
    QString output_;
    int status_;
};

RcloneResult RcloneRPC(QString rc_method, QString rc_input_json) {
    RcloneRPCResult out = RcloneRPC(rc_method.toLocal8Bit().data(),
                                    rc_input_json.toLocal8Bit().data());
    RcloneResult result(out);
    free(out.Output);
    return result;
}

#endif /* QROOKIE_RCLONE_RESULT */
