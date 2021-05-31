/**
 * \internal \file tinywot.c
 * \brief TinyWoT public API implementations.
 *
 * \copyright
 * SPDX-FileCopyrightText: 2021 Junde Yhi <junde@yhi.moe>
 * SPDX-License-Identifier: MIT
 */

#include "tinywot.h"

TinyWoTResponse tinywot_process_request(const TinyWoTConfig *const config,
                                        const TinyWoTThing *const thing,
                                        const TinyWoTRequest *const request) {
  TinyWoTResponse response = {0};

  for (size_t i = 0; i < thing->handlers_size; i++) {
    const TinyWoTHandler *const handler = &(thing->handlers[i]);

    if ((config->strcmp(request->path, handler->path) == 0) &&
        (request->op & handler->ops)) {
      response = handler->func(request);
    }
  }

  if (response.status == TINYWOT_RESPONSE_STATUS_UNKNOWN)
    response.status = TINYWOT_RESPONSE_STATUS_UNSUPPORTED;

  return response;
}
