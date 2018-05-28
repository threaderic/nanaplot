#include "iostream"
#include <nana/gui.hpp>
#include "plot.h"
namespace nana
{
namespace plot
{

plot::plot( window parent )
    : myParent( parent )
{
    // arrange for the plot to be updated when needed
    myDrawing = new drawing( parent );
    myDrawing->draw([this](paint::graphics& graph)
    {
        if( ! myTrace.size() )
            return;

        // calculate scaling factors
        // so plot will fit
        CalcScale(
            graph.width(),
            graph.height() );

        myAxis->update( graph );

        // draw all the traces
        for( auto t : myTrace ) {
            t->update( graph );
        }
    });

    myAxis = new axis( this );
}

void plot::CalcScale( int w, int h )
{
    int maxCount = 0;
    myTrace[0]->bounds( myMinY, myMaxY );
    for( auto& t : myTrace )
    {
        if( t->size() > maxCount )
            maxCount = t->size();
        int tmin, tmax;
        t->bounds( tmin, tmax );
        if( tmin < myMinY )
            myMinY = tmin;
        if( tmax > myMaxY )
            myMaxY = tmax;
    }
    if( ! maxCount )
        return;
    myXinc = (float)w / maxCount;
    if( myMaxY == myMinY )
        myScale = 1;
    else
        myScale = (double) h / ( myMaxY - myMinY );
    myYOffset = (double) h + myScale * myMinY;
}
void trace::set( const std::vector< double >& y )
{
    if( myfRealTime )
        throw std::runtime_error("nanaplot error: static data added to realtime trace");

    myY.clear();
    for( double s : y )
        myY.push_back( s );
    //myY = y;

    std::cout << "plot::trace::set " << myY.size() << "\n";
}
void trace::add( double y )
{
    if( ! myfRealTime )
        throw std::runtime_error("nanaplot error: realtime data added to no realtime trace");
    myY[ myRealTimeNext++ ] = y;
    if( myRealTimeNext >= (int)myY.size() )
        myRealTimeNext = 0;

    myPlot->update();
}

void trace::bounds( int& tmin, int& tmax )
{
    if( ! myY.size() )
        return;
    tmin = myY[0];
   tmax = tmin;
    for( auto y : myY )
    {
        if( y < tmin )
            tmin = y;
        if( y > tmax )
            tmax = y;
    }
    tmin--;
    tmax++;
}

void trace::update( paint::graphics& graph )
{
    bool first = true;
    float x = 0;
    float xinc = myPlot->xinc();
    double s = myPlot->Scale();
    int yOffset = myPlot->YOffset();
    double prev;



    if( ! myfRealTime )
    {
        // loop over data points
        for( auto y : myY )
        {
            // scale
            double ys = yOffset - y * s;

            if( first )
            {
                first = false;
                prev = ys;
                continue;
            }

            // draw line from previous to this data point
            graph.line(
                point(x, prev),
                point(x+xinc, ys),
                myColor);

            x += xinc;
            prev = ys;

        }
    }
    else
    {
        // loop over data points

        // they are stored in a circular buffer
        // so we have to start with the oldest data point
        int yidx = myRealTimeNext;
        do
        {
            double y = yOffset - s * myY[ yidx ];

            // the next data point
            // with wrap-around if the end of the vector is reached
            yidx++;
            if( yidx >= (int)myY.size() )
                yidx = 0;

            if( first )
            {
                first = false;
                prev = y;
                continue;
            }
            // draw line from previous to this data point
            graph.line(
                point( x, prev ),
                point( x+xinc, y ),
                myColor);

            x += xinc;
            prev = y;

        }

        // check for end of circular buffer
        // ( most recent point )
        while( yidx != myRealTimeNext );
    }
}

axis::axis( plot * p )
    : myPlot( p )
{
    myLabelMin = new label( myPlot->parent(),  rectangle{ 10, 10, 50, 50 } );
    myLabelMin->caption("test");
    myLabelMax = new label( myPlot->parent(),  rectangle{ 10, 10, 50, 50 } );
    myLabelMax->caption("test");
}

void axis::update( paint::graphics& graph )
{
    myLabelMin->move( 5,graph.height()-20 );
    std::stringstream ss;
    ss << myPlot->minY();
    myLabelMin->caption(ss.str());
    ss.str("");
    ss << myPlot->maxY();
    myLabelMax->caption(ss.str());
}


}
}
