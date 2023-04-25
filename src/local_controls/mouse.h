/*!
 * This file is part of Lighthouse.
 * COPYRIGHT (C) 2022 Emeric Grange - All Rights Reserved
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * \date      2023
 * \author    Emeric Grange <emeric.grange@gmail.com>
 */

#ifndef MOUSE_H
#define MOUSE_H
/* ************************************************************************** */

#include <QObject>

/* ************************************************************************** */

/*!
 * Minimal API to create virtual mouses.
 */
class Mouse: public QObject
{
    Q_OBJECT

public:
    Mouse(QObject *parent = nullptr);
    virtual ~Mouse() = default;

    virtual void setup() = 0;
    virtual void action(int x, int y, int btn_left, int btn_right) = 0;
};

/* ************************************************************************** */
#endif // MOUSE_H
