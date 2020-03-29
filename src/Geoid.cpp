/*!
 * Copyright (C) 2020 by Johannes Zellner, johannes@zellner.org
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include "Geoid.h"

#if defined(Q_OS_ANDROID)
#include <QAndroidJniObject>
#include <QDebug>

Geoid* Geoid::mInstance = nullptr;
#endif

Geoid::Geoid() : geoidalSeparation(0), isValid(false)
{
#if defined(Q_OS_ANDROID)
    // we need the instance for the JNI Call
    //
    mInstance = this;
#endif
}

float Geoid::operator()() const
{
    return geoidalSeparation;
}

bool Geoid::valid() const
{
    return isValid;
}

#if defined(Q_OS_ANDROID)

Geoid* Geoid::getInstance()
{
    return mInstance;
}

void Geoid::set(float newSeparation)
{
    qDebug() << "Geoid::set " << newSeparation;
    geoidalSeparation = newSeparation;
    isValid = true;
}

#ifdef __cplusplus
extern "C" {
#endif

JNIEXPORT void JNICALL
Java_de_akaflieg_1freiburg_enroute_Geoid_set(JNIEnv* env, jobject, jfloat geoidalSeparation)
{
    Geoid* geoid = Geoid::getInstance();
    if (geoid != nullptr)
    {
        geoid->set(geoidalSeparation);
    }
}

#ifdef __cplusplus
}
#endif

#endif // Q_OS_ANDROID
