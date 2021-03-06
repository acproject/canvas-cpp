/**
 * Copyright (c) 2010 Daniel Wiberg
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Context.h"
#include "binding/Object.h"

#include <iostream>

namespace canvas
{
   
   Context::Context(int width, int height, Canvas::Format format)
      : m_bitmap(0),
        m_device(0),
        m_canvas(0),
        m_parser(),
        m_width(width),
        m_height(height),
        m_format(format)
   {
      m_bitmap = new SkBitmap();
      m_bitmap->setConfig(SkBitmap::kARGB_8888_Config, width, height);
      m_bitmap->allocPixels();
      m_bitmap->eraseARGB(0, 0, 0, 0);
      
      m_device = new SkDevice(*m_bitmap);
      m_canvas = new SkCanvas(m_device);
      
      m_fillPaint.setAntiAlias(true);
      m_fillPaint.setStyle(SkPaint::kFill_Style);
      m_strokePaint.setAntiAlias(true);
      m_strokePaint.setStyle(SkPaint::kStroke_Style);
      
      // Initialize with default state
      m_stateStack.push_back(State());
      syncPaintWithState();
   }
   
   Context::~Context()
   {
   }
   
   void Context::copyImageTo(void * target)
   {
      void * source = m_bitmap->getPixels();
      
      if (m_format == Canvas::kRGBA)
      {
         unsigned char * targetData = static_cast<unsigned char*>(target);
         unsigned char * sourceData = static_cast<unsigned char*>(source);
         for (size_t i = 0; i < m_bitmap->getSize(); i += 4)
         {
            targetData[i + 0] = sourceData[i + 1];
            targetData[i + 1] = sourceData[i + 2];
            targetData[i + 2] = sourceData[i + 3];
            targetData[i + 3] = sourceData[i + 0];
         }
      }
      else
      {
         memcpy(target, source, m_bitmap->getSize());
      }
   }
   
   float Context::lineWidth() const
   {
      return currentState().lineWidth;
   }
   
   void Context::setLineWidth(float width)
   {
      m_strokePaint.setStrokeWidth(width);
      currentState().lineWidth = width;
   }
   
   std::string Context::lineCap() const
   {
      return currentState().lineCapString;
   }
   
   void Context::setLineCap(std::string const& cap)
   {
      currentState().lineCapString = cap;
      m_strokePaint.setStrokeCap(currentState().skiaCap());
   }
   
   std::string Context::strokeStyle() const
   {
      return currentState().strokeStyleString;
   }
   
   void Context::setStrokeStyle(std::string const& style)
   {
      State & state = currentState();
      state.strokeStyleString = style;
      state.strokeStyle = m_parser.parse(style);
      m_strokePaint.setColor(state.strokeStyle.toSkia());
   }
   
   std::string Context::fillStyle() const
   {
      return currentState().fillStyleString;
   }
   
   void Context::setFillStyle(std::string const& style)
   {
      State & state = currentState();
      state.fillStyleString = style;
      state.fillStyle = m_parser.parse(style);
      m_fillPaint.setColor(state.fillStyle.toSkia());
   }
   
   float Context::globalAlpha() const
   {
      return currentState().globalAlpha;
   }
   
   void Context::setGlobalAlpha(float alpha)
   {
      currentState().globalAlpha = alpha;
      m_strokePaint.setAlpha(alpha * 255);
      m_fillPaint.setAlpha(alpha * 255);
   }
   
   void Context::scale(float x, float y)
   {
      m_canvas->scale(x, y);
   }
   
   void Context::rotate(float angle)
   {
      m_canvas->rotate(angle);
   }
   
   void Context::translate(float x, float y)
   {
      m_canvas->translate(x, y);
   }
   
   void Context::drawImage(ImageData * image, float x, float y, float width, float height)
   {
      m_canvas->drawBitmapRect(image->bitmap(), 0, SkRect::MakeLTRB(x, y, x + width, y + height), &m_fillPaint);
   }
   
   v8::Handle<v8::Value> Context::drawImageCallback(v8::Arguments const& args)
   {
      int len = args.Length();
      assert((len == 3 || len == 5) && "Wrong number or arguments to drawImage");
      
      v8::HandleScope scope;
      Context * self = binding::Object<Context>::unwrap(args.Holder());
      ImageData * image = binding::Object<ImageData>::unwrap(args[0]->ToObject());
      
      assert(self && image);
      
      float x = binding::Translate<float>::to(args[1]);
      float y = binding::Translate<float>::to(args[2]);
      float width = len == 3 ? static_cast<float>(image->width()) : binding::Translate<float>::to(args[3]);
      float height = len == 3 ? static_cast<float>(image->height()) : binding::Translate<float>::to(args[4]);
      
      self->drawImage(image, x, y, width, height);
      return v8::Undefined();
   }
   
   void Context::beginPath()
   {
      m_path.rewind();
   }
   
   void Context::closePath()
   {
      
   }

   void Context::fill()
   {
      m_canvas->drawPath(m_path, m_fillPaint);
   }
   
   void Context::stroke()
   {
      m_canvas->drawPath(m_path, m_strokePaint);
   }
   
   void Context::clip()
   {
      
   }

   void Context::moveTo(float x, float y)
   {
      m_path.moveTo(x, y);
   }
   
   void Context::lineTo(float x, float y)
   {
      m_path.lineTo(x, y);
   }
   
   void Context::quadraticCurveTo(float cpx, float cpy, float x, float y)
   {
      m_path.quadTo(cpx, cpy, x, y);
   }
   
   void Context::bezierCurveTo(float cp1x, float cp1y, float cp2x, float cp2y, float x, float y)
   {
      m_path.cubicTo(cp1x, cp1y, cp2x, cp2y, x, y);
   }
   
   void Context::arcTo(float x1, float y1, float x2, float y2, float radius)
   {
      m_path.arcTo(x1, y1, x2, y2, radius);
   }
   
   void Context::rect(float x, float y, float width, float height)
   {
      m_path.addRect(x, y, x + width, y + height);
   }
   
   void Context::fillRect(float x, float y, float width, float height)
   {
      m_canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), m_fillPaint);
   }
   
   void Context::strokeRect(float x, float y, float width, float height)
   {
      m_canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), m_strokePaint);
   }
   
   void Context::clear()
   {
      m_bitmap->eraseARGB(0, 0, 0, 0);
   }
   
   void Context::clearRect(float x, float y, float width, float height)
   {
      SkPaint paint;
      paint.setColor(SkColorSetARGB(0, 0, 0,0));
      m_canvas->drawRect(SkRect::MakeXYWH(x, y, width, height), paint);
   }
   
   void Context::syncPaintWithState()
   {
      State const& state = currentState();
      
      m_fillPaint.setColor(state.fillStyle.toSkia());
      m_fillPaint.setAlpha(state.globalAlpha * 255);
      
      m_strokePaint.setColor(state.strokeStyle.toSkia());
      m_strokePaint.setAlpha(state.globalAlpha * 255);
      m_strokePaint.setStrokeWidth(state.lineWidth);
      m_strokePaint.setStrokeCap(state.skiaCap());
   }
}

