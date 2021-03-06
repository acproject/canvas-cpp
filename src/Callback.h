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

#ifndef CANVAS_CALLBACK_HEADER
#define CANVAS_CALLBACK_HEADER

#include <v8.h>

namespace canvas
{
   class Callback
   {
      public:
      
         Callback(v8::Handle<v8::Function> func, int interval)
            : m_interval(static_cast<double>(interval) / 1000.0),
              m_current(0.0)
         {
            m_function = v8::Persistent<v8::Function>::New(func);
         }
         
         v8::Handle<v8::Value> call(double dt)
         {
            m_current += dt;
            
            if (m_current > m_interval)
            {
               m_current -= m_interval;
               return m_function->Call(m_function, 0, NULL);
            }
            
            return v8::Undefined();
         }
         
      private:
      
         v8::Persistent<v8::Function>  m_function;
         double m_interval;
         double m_current;
   };
}

#endif

