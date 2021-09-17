/*
 *  Copyright (C) 2015-2021 Team Kodi (https://kodi.tv)
 *  Copyright (C) 2015 Sam Stenvall
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include "../request/Request.h"
#include "Response.h"

#include <memory>

namespace vbox
{
  namespace response
  {

    /**
     * Factor for response objects
     */
    class Factory
    {
    public:
      /**
      * Prevents construction
      */
      Factory() = delete;

      /**
       * Factory method for creating response objects
       * @param request the request
       * @return the response
       */
      static ResponsePtr CreateResponse(const request::Request& request)
      {
        switch (request.GetResponseType())
        {
          case ResponseType::XMLTV:
            return ResponsePtr(new XMLTVResponse);
          case ResponseType::RECORDS:
            return ResponsePtr(new RecordingResponse);
          case ResponseType::GENERIC:
          default:
            return ResponsePtr(new Response);
        }
      }
    };
  } // namespace response
} // namespace vbox
