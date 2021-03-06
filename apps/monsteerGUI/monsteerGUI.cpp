/* Copyright (c) 2011-2015, EPFL/Blue Brain Project
 *                          Jafet Villafranca <jafet.villafrancadiaz@epfl.ch>
 *                          Ahmet Bilgili <ahmet.bilgili@epfl.ch>
 *                          Maxim Makhinya <maxmah@gmail.com>
 *                          Stefan.Eilemann@epfl.ch
 *
 * This file is part of Monsteer <https://github.com/BlueBrain/Monsteer>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License version 3.0 as published
 * by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include <monsteer/qt/SteeringWidget.h>

#include <QApplication>

namespace
{
static const servus::URI LIVRE_ZEQ_SCHEMA( "hbp://" );
static const servus::URI NEST_ZEQ_SCHEMA( "monsteer-nesteer://" );
}

int main( int argc, char *argv[] )
{
    QApplication app( argc, argv );

    std::unique_ptr< QWidget > widget(
        new monsteer::qt::SteeringWidget( LIVRE_ZEQ_SCHEMA, NEST_ZEQ_SCHEMA ));
    widget->show();

    return app.exec();
}
