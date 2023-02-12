/*
  SPDX-FileCopyrightText: 2021-2023 Junde Yhi <junde@yhi.moe>
  SPDX-License-Identifier: MIT
*/

/*!
  \internal \file
  \brief TinyWoT public API implementations.
*/

#include "base.h"
#include <string.h>

#include <tinywot.h>

struct tinywot_scratchpad tinywot_scratchpad_new(void)
{
  return (struct tinywot_scratchpad) {
    .size = 0u,
    .valid_size = 0u,
    .type_hint = TINYWOT_TYPE_UNKNOWN,
    .data = NULL,
  };
}

struct tinywot_scratchpad
tinywot_scratchpad_new_with_empty_memory(void *ptr, size_t size)
{
  return (struct tinywot_scratchpad) {
    .size = size,
    .valid_size = 0u,
    .type_hint = TINYWOT_TYPE_UNKNOWN,
    .data = ptr,
  };
}

struct tinywot_scratchpad tinywot_scratchpad_new_with_used_memory(
  void *ptr, size_t size, size_t valid_size, unsigned int type_hint
)
{
  return (struct tinywot_scratchpad) {
    .size = size,
    .valid_size = valid_size,
    .type_hint = type_hint,
    .data = ptr,
  };
}

int tinywot_thing_get_handler_function(
  struct tinywot_thing const *self,
  char const *path,
  enum tinywot_operation_type op,
  tinywot_handler_function_t **func,
  void **user_data
)
{
  int status = TINYWOT_ERROR_NOT_FOUND;

  for (size_t i = 0; i < self->handlers_size; i += 1) {
    if (tinywot_strcmp(path, self->handlers[i].path) != 0) {
      continue;
    }

    /* We do not allow an unknown / uninitialized operation type in a thing. */
    if (op != self->handlers[i].op || op == TINYWOT_OPERATION_TYPE_UNKNOWN) {
      status = TINYWOT_ERROR_NOT_ALLOWED;
      continue;
    }

    /* A user can choose to not retrieve the pointers if they only want to
       know whether the handler exists (by checking the return code). */
    if (func) {
      *func = self->handlers[i].func;
    }

    if (user_data) {
      *user_data = self->handlers[i].user_data;
    }

    status = TINYWOT_SUCCESS;

    /* No security consideration for this naive implementation. */
    break;
  }

  return status;
}

int tinywot_thing_do(
  struct tinywot_thing const *self,
  char const *path,
  enum tinywot_operation_type op,
  struct tinywot_scratchpad const *input,
  struct tinywot_scratchpad *output
)
{
  tinywot_handler_function_t *func = NULL;
  void *user_data = NULL;
  int status = 0;

  status
    = tinywot_thing_get_handler_function(self, path, op, &func, &user_data);

  if (status != TINYWOT_SUCCESS) {
    return status;
  }

  /* We allow null pointer here to allow an entry behave like a stub. */
  if (!func) {
    return TINYWOT_ERROR_NOT_IMPLEMENTED;
  }

  /* `input` or `output` may be null, which the handler function should be
     aware of. */
  return func(input, output, user_data);
}

int tinywot_thing_read_property(
  struct tinywot_thing const *self,
  char const *path,
  struct tinywot_scratchpad *output
)
{
  return tinywot_thing_do(
    self, path, TINYWOT_OPERATION_TYPE_READPROPERTY, NULL, output
  );
}

int tinywot_thing_write_property(
  struct tinywot_thing const *self,
  char const *path,
  struct tinywot_scratchpad const *input
)
{
  return tinywot_thing_do(
    self, path, TINYWOT_OPERATION_TYPE_WRITEPROPERTY, input, NULL
  );
}

int tinywot_thing_invoke_action(
  struct tinywot_thing const *self,
  char const *path,
  struct tinywot_scratchpad const *input
)
{
  return tinywot_thing_do(
    self, path, TINYWOT_OPERATION_TYPE_INVOKEACTION, input, NULL
  );
}

int tinywot_thing_process_request(
  struct tinywot_thing const *self,
  struct tinywot_request const *request,
  struct tinywot_response *response
)
{
  int status = 0;

  status = tinywot_thing_do(
    self, request->path, request->op, request->content, response->content
  );

  /* Map TinyWoT status codes to response status codes. */
  switch (status) {
    case TINYWOT_SUCCESS:
      response->status = TINYWOT_RESPONSE_STATUS_OK;
      break;

    case TINYWOT_ERROR_NOT_FOUND:
      response->status = TINYWOT_RESPONSE_STATUS_NOT_FOUND;
      break;

    case TINYWOT_ERROR_NOT_IMPLEMENTED:
      response->status = TINYWOT_RESPONSE_STATUS_NOT_SUPPORTED;
      break;

    case TINYWOT_ERROR_NOT_ALLOWED:
      response->status = TINYWOT_RESPONSE_STATUS_NOT_ALLOWED;
      break;

    case TINYWOT_ERROR_GENERAL_ERROR:
      /* fall through */

    default:
      response->status = TINYWOT_RESPONSE_STATUS_ERROR;
      break;
  }

  return status;
}

int tinywot_servient_run(struct tinywot_servient const *self)
{
  (void)self;

  /* TODO: Not implemented yet! */
  return TINYWOT_ERROR_NOT_IMPLEMENTED;
}
