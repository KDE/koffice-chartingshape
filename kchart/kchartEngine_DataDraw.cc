#include "engine.h"
#include "engine2.h"
#include "enginehelper.h"
#include "enginedraw.h"

#include "kchartparams.h"
#include "kchartEngine.h"

#include <qfont.h>
#include <qcolor.h>
#include <qpainter.h>

#include <stdlib.h>
#include <stdarg.h>
#include <math.h>
#include <kdebug.h>


void KChartEngine::drawData() {
  switch( params->type ) {
  case KCHARTTYPE_3DBAR: // depth, width, y interval need to allow for whitespace between bars
  case KCHARTTYPE_BAR:
    kdDebug(35001) << "drawing bars" << endl;
    drawBars();
    break;
  case KCHARTTYPE_LINE:
  case KCHARTTYPE_COMBO_LINE_BAR:
  case KCHARTTYPE_COMBO_LINE_AREA:
    kdDebug(35001) << "drawing lines" << endl;
    drawLines();
    if( params->cross)
      {
		kdDebug(35001) << "drawing cross" << endl;
        drawCross();
      }

    break;

  case KCHARTTYPE_3DLINE:
  case KCHARTTYPE_3DCOMBO_LINE_BAR:
  case KCHARTTYPE_3DCOMBO_LINE_AREA:
    kdDebug(35001) << "drawing 3D lines" << endl;
    draw3DLines();
    break;
  case KCHARTTYPE_AREA:
  case KCHARTTYPE_3DAREA:
    kdDebug(35001) << "drawing areas" << endl;
    drawArea();
    break;
  case KCHARTTYPE_3DHILOCLOSE:
  case KCHARTTYPE_3DCOMBO_HLC_BAR:
  case KCHARTTYPE_3DCOMBO_HLC_AREA:
    kdDebug(35001) << "drawing 3D combos" << endl;
    draw3DCombo();
    break;
  case KCHARTTYPE_HILOCLOSE:
  case KCHARTTYPE_COMBO_HLC_BAR:
  case KCHARTTYPE_COMBO_HLC_AREA:
    kdDebug(35001) << "drawing combos" << endl;
    drawCombo();
    break;

  case KCHARTTYPE_3DPIE:
  case KCHARTTYPE_2DPIE:
    kdDebug(35001) << "drawing pie" << endl;
    drawPie();
    break;
  }
}

void KChartEngine::drawBars() {
  int i, j;
  switch( params->stack_type ) {
  case KCHARTSTACKTYPE_DEPTH:
    for( setno=num_sets-1; setno>=0; --setno )		// back sets first   PX, PY depth
      for(int i=0; i<num_points; ++i ) {
		if( CELLEXISTS( setno, i ) )
		  draw_3d_bar( p, PX(i+(params->do_bar()?1:0))-hlf_barwdth, PX(i+(params->do_bar()?1:0))+hlf_barwdth,
					   PY(0), PY( CELLVALUE( setno, i )),
					   xdepth_3D, ydepth_3D,
					   ExtColor[setno][i],
					   params->threeD()? ExtColorShd[setno][i]: ExtColor[setno][i] );
      }
    setno = 0;
    break;

  case KCHARTSTACKTYPE_LAYER:
    {
      float	*lasty = new float[num_points];
      j = 0;
      //for( i=0; i<num_points; ++i )
      //if( CELLEXISTS( j, i ) ) {
	  //lasty[i] = CELLVALUE( j, i );
	  //draw_3d_bar( im, PX(i+(params->do_bar()?1:0))-hlf_barwdth, PX(i+(params->do_bar()?1:0))+hlf_barwdth,
	  //PY(0), PY(CELLVALUE( j, i )),
	  //xdepth_3D, ydepth_3D,
	  //ExtColor[j][i],
	  //params->threeD()? ExtColorShd[j][i]: ExtColor[j][i] );
	  //}
      for( i=0; i<num_points; ++i ) {
		struct BS	*barset = new struct BS[num_sets];
		float		lasty_pos = 0.0;
		float		lasty_neg = 0.0;
		int			k;

		for( j=0, k=0; j<num_sets; ++j ) {
		  if( CELLEXISTS( j, i ) ) {
			if( CELLVALUE( j, i ) < 0.0 ) {
			  barset[k].y1 = lasty_neg;
			  barset[k].y2 = CELLVALUE( j, i ) + lasty_neg;
			  lasty_neg    = barset[k].y2;
			} else {
			  barset[k].y1 = lasty_pos;
			  barset[k].y2 = CELLVALUE( j, i ) + lasty_pos;
			  lasty_pos    = barset[k].y2;
			}
			barset[k].clr   = ExtColor[j][i];
			barset[k].shclr = params->threeD()? ExtColorShd[j][i]: ExtColor[j][i];
			++k;
		  }
		}
		qsort( barset, k, sizeof(struct BS), barcmpr );

		for( j=0; j<k; ++j ) {
		  draw_3d_bar( p,
					   PX(i+(params->do_bar()?1:0))-hlf_barwdth, PX(i+(params->do_bar()?1:0))+hlf_barwdth,
					   PY(barset[j].y1), PY(barset[j].y2),
					   xdepth_3D, ydepth_3D,
					   barset[j].clr,
					   barset[j].shclr );
		}
		delete [] barset;
      }
      delete [] lasty;
    }
    break;

  case KCHARTSTACKTYPE_BESIDE:
    {												// h/.5, h/1, h/1.5, h/2, ...
      int	new_barwdth = (int)( (float)hlf_barwdth / ((float)num_sets/2.0) );
      for( i=0; i<num_points; ++i )
		for( j=0; j<num_sets; ++j )
		  if( CELLEXISTS( j, i ) )
			draw_3d_bar( p, PX(i+(params->do_bar()?1:0))-hlf_barwdth+new_barwdth*j+1,
						 PX(i+(params->do_bar()?1:0))-hlf_barwdth+new_barwdth*(j+1),
						 PY(0), PY(CELLVALUE( j, i )),
						 xdepth_3D, ydepth_3D,
						 ExtColor[j][i],
						 params->threeD()? ExtColorShd[j][i]: ExtColor[j][i] );
    }
    break;



  case KCHARTSTACKTYPE_PERCENT:
    {
      float	*lasty = new float[num_points];
      j = 0;

      float * coef = new float[num_points];
      for(j=0; j<num_points; ++j )
		{
		  float sum = 0.0;
		  for(int i=0; i<num_sets; ++i )
			{
			  if( CELLEXISTS( i, j ) )
				if( CELLVALUE( i, j ) < 0.0 )
				  sum -= CELLVALUE( i, j );
				else
				  sum += CELLVALUE( i, j );
			}
		  coef[j]=sum;
		}
      for( i=0; i<num_points; ++i ) {
		struct BS	*barset = new struct BS[num_sets];
		float		lasty_pos = 0.0;
		float		lasty_neg = 0.0;
		int			k;

		for( j=0, k=0; j<num_sets; ++j ) {
		  if( CELLEXISTS( j, i ) )
			{
			  if( CELLVALUE( j, i ) < 0.0 )
				{
				  barset[k].y1 = lasty_neg;
				  barset[k].y2 = CELLVALUE( j, i )*100/coef[i] + lasty_neg;
				  lasty_neg    = barset[k].y2;
				}
			  else {
				barset[k].y1 = lasty_pos;
				barset[k].y2 = CELLVALUE( j, i )*100/coef[i] + lasty_pos;
				lasty_pos    = barset[k].y2;
			  }
			  barset[k].clr   = ExtColor[j][i];
			  barset[k].shclr = params->threeD()? ExtColorShd[j][i]: ExtColor[j][i];
			  ++k;
			}
		}
		qsort( barset, k, sizeof(struct BS), barcmpr );

		for( j=0; j<k; ++j ) {
		  draw_3d_bar( p,
					   PX(i+(params->do_bar()?1:0))-hlf_barwdth, PX(i+(params->do_bar()?1:0))+hlf_barwdth,
					   PY(barset[j].y1), PY(barset[j].y2),
					   xdepth_3D, ydepth_3D,
					   barset[j].clr,
					   barset[j].shclr );
		}

		delete [] barset;
      }
      delete [] lasty;
      delete [] coef;
    }
    break;

  case KCHARTSTACKTYPE_SUM:
    kdDebug( 35001 ) << "Sorry, not implemented" << endl;
    break;
  }
}


void KChartEngine::drawLines()
{
  for(int j=num_sets-1; j>=0; --j )
    {
      for(int i=1; i<num_points; ++i )
        {
		  if( CELLEXISTS(j,i-1) && CELLEXISTS(j,i) )
			{
			  p->setPen( ExtColor[j][i] );
			  p->drawLine( PX(i-1), PY(CELLVALUE( j, i-1 )), PX(i), PY(CELLVALUE( j, i )) );
			  p->drawLine( PX(i-1), PY(CELLVALUE(j,i-1))+1, PX(i), PY(CELLVALUE(j,i))+1 );
			}
		  else
			{
			  if( CELLEXISTS(j,i-1) )
				{
				  p->setPen( ExtColor[j][i] );
				  p->drawPoint( PX(i-1), PY(CELLVALUE(j,i-1)) );
				}
			  if( CELLEXISTS(j,i) )
				{
				  p->setPen( ExtColor[j][i] );
				  p->drawPoint( PX(i), PY(CELLVALUE(j,i)) );
				}
			}
        }
    }
}

void KChartEngine::drawCross()
{
  for(int j=num_sets-1; j>=0; --j )
    {
      for(int i=0; i<num_points; ++i )
        {
		  if( CELLEXISTS(j,i)  )
			{
			  p->setPen( ExtColor[j][i] );
			  p->drawLine( PX(i), PY(CELLVALUE( j, i ))-8, PX(i), PY(CELLVALUE( j, i ))+8);
			  p->drawLine( PX(i)-8, PY(CELLVALUE(j,i)), PX(i)+8, PY(CELLVALUE(j,i)) );
			}

        }
    }
}

void KChartEngine::draw3DLines()
{
  int *y1 = new int[num_sets];
  int *y2 = new int[num_sets];

  for(int i=1; i<num_points; ++i )
    {
      if( params->stack_type == KCHARTSTACKTYPE_DEPTH ) {
		for(int j=num_sets-1; j>=0; --j ) {
		  if( CELLEXISTS(j,i-1) && CELLEXISTS(j,i) )
			{
			  setno = j;
			  y1[j] = PY(CELLVALUE(j,i-1));
			  y2[j] = PY(CELLVALUE(j,i));

			  draw_3d_line( p,
							PY(0),
							PX(i-1), PX(i),
							&(y1[j]), &(y2[j]),
							xdepth_3D, ydepth_3D,
							1,
							&(ExtColor[j][i]),
							&(ExtColorShd[j][i]) );
			  setno = 0;
			}
		}
      }
      else if( params->stack_type == KCHARTSTACKTYPE_BESIDE ||
			   params->stack_type == KCHARTSTACKTYPE_SUM )
		{// all same plane
		  int set=0;
		  QColor *clr = new QColor[num_sets];
          QColor *clrshd = new QColor[num_sets];
		  float usey1 = 0.0;
		  float usey2 = 0.0;
		  for(int j=0,set=0; j<num_sets; ++j ) {
			if( CELLEXISTS(j,i-1) && CELLEXISTS(j,i) ) {
			  if( params->stack_type == KCHARTSTACKTYPE_SUM ) {
				usey1 += CELLVALUE(j,i-1);
				usey2 += CELLVALUE(j,i);
			  }
			  else
				{
				  usey1 = CELLVALUE(j,i-1);
				  usey2 = CELLVALUE(j,i);
				}
			  y1[set]     = PY(usey1);
			  y2[set]     = PY(usey2);
			  clr[set]    = ExtColor[j][i];
			  clrshd[set] = ExtColorShd[j][i];	/* fred */
			  ++set;
			}
		  }
		  draw_3d_line( p,
						PY(0),
						PX(i-1), PX(i),
						y1, y2,
						xdepth_3D, ydepth_3D,
						set,
						clr,
						clrshd );
          delete [] clr;
          delete [] clrshd;
		}
    }
  delete [] y1;
  delete [] y2;
}


void KChartEngine::drawArea() {
  int i=0,j=0;
  switch( params->stack_type )
    {
    case KCHARTSTACKTYPE_SUM:
      {
		float  *lasty = new float[num_points];
		j = 0;
		for( i=1; i<num_points; ++i )
		  if( CELLEXISTS(j,i) )
			{
			  lasty[i] = CELLVALUE(j,i);
			  if( CELLEXISTS(j,i-1) )
				draw_3d_area( p, PX(i-1), PX(i),
							  PY(0), PY(CELLVALUE(j,i-1)), PY(CELLVALUE(j,i)),
							  xdepth_3D, ydepth_3D,
							  ExtColor[j][i],
							  params->threeD()? ExtColorShd[j][i]: ExtColor[j][i] );
			}
		for( j=1; j<num_sets; ++j )
		  for( i=1; i<num_points; ++i )
			if( CELLEXISTS(j,i) &&
				CELLEXISTS(j,i-1) ) {
			  draw_3d_area( p, PX(i-1), PX(i),
							PY(lasty[i]), PY(lasty[i-1]+CELLVALUE(j,i-1)), PY(lasty[i]+CELLVALUE(j,i)),
							xdepth_3D, ydepth_3D,
							ExtColor[j][i],
							params->threeD()? ExtColorShd[j][i]: ExtColor[j][i] );
			  lasty[i] += CELLVALUE(j,i);
			}
        delete [] lasty;
      }
      break;

    case KCHARTSTACKTYPE_BESIDE:								// behind w/o depth
      for( j=num_sets-1; j>=0; --j )					// back sets 1st  (setno = 0)
		for( i=1; i<num_points; ++i )
		  if( CELLEXISTS(j,i-1) && CELLEXISTS(j,i) )
			draw_3d_area( p, PX(i-1), PX(i),
						  PY(0), PY(CELLVALUE(j,i-1)), PY(CELLVALUE(j,i)),
						  xdepth_3D, ydepth_3D,
						  ExtColor[j][i],
						  params->threeD()? ExtColorShd[j][i]: ExtColor[j][i] );
      break;


    case KCHARTSTACKTYPE_DEPTH:
    default:
      for( setno=num_sets-1; setno>=0; --setno )		// back sets first   PX, PY depth
		for( i=1; i<num_points; ++i )
		  if( CELLEXISTS(setno,i-1) &&
			  CELLEXISTS(setno,i) )
			draw_3d_area( p, PX(i-1), PX(i),
						  PY(0), PY(CELLVALUE(setno,i-1)), PY(CELLVALUE(setno,i)),
						  xdepth_3D, ydepth_3D,
						  ExtColor[setno][i],
						  params->threeD()? ExtColorShd[setno][i]: ExtColor[setno][i] );
      setno = 0;
    }
}

void KChartEngine::drawCombo() {
  int i=0;
  int j=0;
  for(j=num_hlc_sets-1; j>=0; --j )
	{
	  for( i=0; i<num_points; ++i )
		{
		  if(((CLOSESET+j*3)< num_sets)&&CELLEXISTS(CLOSESET+j*3,i) )
			{ /* all HLC have vert line */
			  if((LOWSET+j*3) <num_sets)
                {
				  if(CELLEXISTS(LOWSET+j*3,i) )
					{
					  p->setPen( ExtColor[LOWSET+(j*3)][i] );
					  p->drawLine( PX(i), PY(CELLVALUE(CLOSESET+j*3,i)),
								   PX(i), PY(CELLVALUE(LOWSET+j*3,i)) );
					}
				}
			  if((HIGHSET+j*3) <num_sets)
                {
				  if(CELLEXISTS(HIGHSET+j*3,i) )
					{
					  p->setPen( ExtColor[HIGHSET+j*3][i] );
					  p->drawLine( PX(i), PY(CELLVALUE(HIGHSET+j*3,i)),
								   PX(i), PY(CELLVALUE(CLOSESET+j*3,i)) );
					}
				}

			  if( params->hlc_style == KCHARTHLCSTYLE_ICAP )
                {
				  if((LOWSET+j*3)<num_sets )
					{
					  if(CELLEXISTS(LOWSET+j*3,i) )
						{
						  p->setPen( ExtColor[LOWSET+j*3][i] );
						  p->drawLine( PX(i)-hlf_hlccapwdth, PY(CELLVALUE(LOWSET+j*3,i)),
									   PX(i)+hlf_hlccapwdth, PY(CELLVALUE(LOWSET+j*3,i)) );
						}
	                }
				  if((HIGHSET+j*3)<num_sets)
					{
					  if(CELLEXISTS(HIGHSET+j*3,i))
						{
						  p->setPen( ExtColor[HIGHSET+j*3][i] );
						  p->drawLine( PX(i)-hlf_hlccapwdth, PY(CELLVALUE(HIGHSET+j*3,i)),
									   PX(i)+hlf_hlccapwdth, PY(CELLVALUE(HIGHSET+j*3,i)) );
						}
	                }
				}
			  if( params->hlc_style == KCHARTHLCSTYLE_DIAMOND )
                {
				  QPointArray cd( 4 );
				  cd.setPoint( 0, PX(i)-hlf_hlccapwdth, PY(CELLVALUE(CLOSESET+j*3,i) ) );
				  cd.setPoint( 1, PX(i), PY(CELLVALUE(CLOSESET+j*3,i))+hlf_hlccapwdth );
				  cd.setPoint( 2, PX(i)+hlf_hlccapwdth, PY(CELLVALUE(CLOSESET+j*3,i) ) );
				  cd.setPoint( 3, PX(i), PY(CELLVALUE(CLOSESET+j*3,i))-hlf_hlccapwdth );
				  p->setBrush( QBrush( ExtColor[CLOSESET+j*3][i] ) );
				  p->setPen( ExtColor[CLOSESET+j*3][i] );
				  p->drawPolygon( cd );

				}
			}
        }
	  for( i=1; i<num_points; ++i )
        {
		  if((CLOSESET+j*3)<num_sets )
			{
			  if(CELLEXISTS(CLOSESET+j*3,i-1) && CELLEXISTS(CLOSESET+j*3,i) )
				{
				  if( params->hlc_style == KCHARTHLCSTYLE_CLOSECONNECTED )
					{// line from prev close
					  p->setPen( ExtColor[CLOSESET+j*3][i] );
					  p->drawLine( PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1)),
								   PX(i), PY(CELLVALUE(CLOSESET+j*3,i-1)) );
					}
				  else// CLOSE_CONNECTED and CONNECTING are mutually exclusive
					{
					  if( params->hlc_style == KCHARTHLCSTYLE_CONNECTING )
                        {// thin connecting line
						  p->setPen( ExtColor[CLOSESET+j*3][i] );
						  p->drawLine( PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1)),
									   PX(i), PY(CELLVALUE(CLOSESET+j*3,i)) );
						}
					}
				}
			}
		}
	}
}


void KChartEngine::draw3DCombo() {
  QPointArray  poly( 4 );
  int i=0;
  int j=0;
  for( j=num_hlc_sets-1; j>=0; --j )
	{
	  for( i=1; i<num_points+1; ++i )
		if(((CLOSESET+j*3)<num_sets )&& CELLEXISTS(CLOSESET+j*3,i-1) )
		  {
			if( (params->hlc_style == KCHARTHLCSTYLE_ICAP) &&// bottom half of 'I'
				((LOWSET+j*3)<num_sets)&& CELLEXISTS(LOWSET+j*3,i-1) )
			  {
				set3dpoly( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
						   PY(CELLVALUE(LOWSET+j*3,i-1)), PY(CELLVALUE(LOWSET+j*3,i-1)),
						   xdepth_3D, ydepth_3D );
				p->setBrush( QBrush( ExtColor[LOWSET+j*3][i-1] ) );
				p->setPen( ExtColor[LOWSET+j*3][i-1] );
				p->drawPolygon( poly );
				p->setBrush( QBrush( ExtColorShd[LOWSET+j*3][i-1], Qt::NoBrush ) );
				p->setPen( ExtColorShd[LOWSET+j*3][i-1] );
				p->drawPolygon( poly );
			  }
			// all HLC have vertical line
			if( ((LOWSET+j*3)<num_sets ) && CELLEXISTS(LOWSET+j*3,i-1) )
			  {// bottom 'half'
				set3dpoly( poly, PX(i-1), PX(i-1),
						   PY(CELLVALUE(LOWSET+j*3,i-1)), PY(CELLVALUE(CLOSESET+j*3,i-1)),
						   xdepth_3D, ydepth_3D );
				p->setBrush( QBrush( ExtColor[LOWSET+j*3][i-1] ) );
				p->setPen( ExtColor[LOWSET+j*3][i-1] );
				p->drawPolygon( poly );
				p->setBrush( QBrush( ExtColorShd[LOWSET+j*3][i-1], Qt::NoBrush ) );
				p->setPen( ExtColorShd[LOWSET+j*3][i-1] );
				p->drawPolygon( poly );
			  }
			if( ((HIGHSET+j*3)<num_sets ) && CELLEXISTS(HIGHSET+j*3,i-1 ) )
			  {// top 'half'
				set3dpoly( poly, PX(i-1), PX(i-1),
						   PY(CELLVALUE(CLOSESET+j*3,i-1)), PY(CELLVALUE(HIGHSET+j*3,i-1)),
						   xdepth_3D, ydepth_3D );
				p->setBrush( QBrush( ExtColor[HIGHSET+j*3][i-1] ) );
				p->setPen( ExtColor[HIGHSET+j*3][i-1] );
				p->drawPolygon( poly );
				p->setBrush( QBrush( ExtColorShd[HIGHSET+j*3][i-1], Qt::NoBrush ) );
				p->setPen( ExtColorShd[HIGHSET+j*3][i-1] );
				p->drawPolygon( poly );
			  }
			// line at close
			p->setPen( ExtColorShd[CLOSESET+j*3][i-1] );
			p->drawLine( PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1)),
						 PX(i-1)+xdepth_3D, PY(CELLVALUE(CLOSESET+j*3,i-1))-ydepth_3D );
			// top half 'I'
			if( !( (params->hlc_style == KCHARTHLCSTYLE_DIAMOND) &&
				   ((HIGHSET+j*3)<num_sets )&&  ((CLOSESET+j*3)<num_sets )&&
				   (PY(CELLVALUE(HIGHSET+j*3,i-1)) > PY(CELLVALUE(CLOSESET+j*3,i-1))-hlf_hlccapwdth) ) &&
				CELLEXISTS(HIGHSET+j*3,i-1) )
			  if( params->hlc_style == KCHARTHLCSTYLE_ICAP )
                {
				  set3dpoly( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
							 PY(CELLVALUE(HIGHSET+j*3,i-1)), PY(CELLVALUE(HIGHSET+j*3,i-1)),
							 xdepth_3D, ydepth_3D );
				  p->setBrush( QBrush( ExtColor[HIGHSET+j*3][i-1] ) );
				  p->setPen( ExtColor[HIGHSET+j*3][i-1] );
				  p->drawPolygon( poly );
				  p->setBrush( QBrush( ExtColorShd[HIGHSET+j*3][i-1], Qt::NoBrush ) );
				  p->setPen( ExtColorShd[HIGHSET+j*3][i-1] );
				  p->drawPolygon( poly );
				}

			if( i < num_points && ((CLOSESET+j*3)<num_sets)&& CELLEXISTS(CLOSESET+j*3,i) )
			  {
				if( params->hlc_style == KCHARTHLCSTYLE_CLOSECONNECTED )/* line from prev close */
				  {
					set3dpoly( poly, PX(i-1), PX(i),
							   PY(CELLVALUE(CLOSESET+j*3,i-1)), PY(CELLVALUE(CLOSESET+j*3,i-1)),
							   xdepth_3D, ydepth_3D );
					p->setBrush( QBrush( ExtColor[CLOSESET+j*3][i] ) );
					p->setPen( ExtColor[CLOSESET+j*3][i] );
					p->drawPolygon( poly );
					p->setBrush( QBrush( ExtColorShd[CLOSESET+j*3][i], Qt::NoBrush ) );
					p->setPen( ExtColorShd[CLOSESET+j*3][i] );
					p->drawPolygon( poly );
				  }
				else	// CLOSE_CONNECTED and CONNECTING are mutually exclusive
				  if( params->hlc_style == KCHARTHLCSTYLE_CONNECTING )
					{	/* thin connecting line */
					  int y1 = PY(CELLVALUE(CLOSESET+j*3,i-1));
					  int y2 = PY(CELLVALUE(CLOSESET+j*3,i));
					  draw_3d_line( p,
									PY(0),
									PX(i-1), PX(i),
									&y1, &y2,					// rem only 1 set
									xdepth_3D, ydepth_3D,
									1,
									&(ExtColor[CLOSESET+j*3][i]),
									&(ExtColorShd[CLOSESET+j*3][i]) );
					  // edge font of it
					  p->setPen( ExtColorShd[CLOSESET+j*3][i] );
					  p->drawLine( PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1)),
								   PX(i), PY(CELLVALUE(CLOSESET+j*3,i)) );
					}
				// top half 'I' again
				if( PY(CELLVALUE(CLOSESET+j*3,i-1)) <= PY(CELLVALUE(CLOSESET+j*3,i)) &&
					((HIGHSET+j*3)<num_sets)&&CELLEXISTS(HIGHSET+j*3,i-1) )
				  if( params->hlc_style & KCHARTHLCSTYLE_ICAP ) {
					set3dpoly( poly, PX(i-1)-hlf_hlccapwdth, PX(i-1)+hlf_hlccapwdth,
							   PY(CELLVALUE(HIGHSET+j*3,i-1)), PY(CELLVALUE(HIGHSET+j*3,i-1)),
							   xdepth_3D, ydepth_3D );
					p->setBrush( QBrush( ExtColor[HIGHSET+j*3][i-1] ) );
					p->setPen( ExtColor[HIGHSET+j*3][i-1] );
					p->drawPolygon( poly );
					p->setBrush( QBrush( ExtColorShd[HIGHSET+j*3][i-1], Qt::NoBrush ) );
					p->setPen( ExtColorShd[HIGHSET+j*3][i-1] );
					p->drawPolygon( poly );
				  }
			  }
			if( params->hlc_style == KCHARTHLCSTYLE_DIAMOND &&((CLOSESET+j*3)<num_sets) )
			  { // front
				poly.setPoint( 0, PX(i-1)-hlf_hlccapwdth,
							   PY(CELLVALUE(CLOSESET+j*3,i-1) ) );
				poly.setPoint( 1, PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1))+hlf_hlccapwdth );
				poly.setPoint( 2, PX(i-1)+hlf_hlccapwdth, PY(CELLVALUE(CLOSESET+j*3,i-1) ) );
				poly.setPoint( 3, PX(i-1), PY(CELLVALUE(CLOSESET+j*3,i-1))-hlf_hlccapwdth );
				p->setBrush( QBrush( ExtColor[CLOSESET+j*3][i-1] ) );
				p->setPen( ExtColor[CLOSESET+j*3][i-1] );
				p->drawPolygon( poly );
				p->setBrush( QBrush( ExtColorShd[CLOSESET+j*3][i-1], Qt::NoBrush ) );
				p->setPen( ExtColorShd[CLOSESET+j*3][i-1] );
				p->drawPolygon( poly );
				// bottom side
				set3dpoly( poly, PX(i-1), PX(i-1)+hlf_hlccapwdth,
						   PY(CELLVALUE(CLOSESET+j*3,i-1))+hlf_hlccapwdth,
						   PY(CELLVALUE(CLOSESET+j*3,i-1)),
						   xdepth_3D, ydepth_3D );
				p->setBrush( QBrush( ExtColorShd[CLOSESET+j*3][i-1] ) );
				p->setPen( ExtColorShd[CLOSESET+j*3][i-1] );
				p->drawPolygon( poly );

				// gdImagePolygon( im, poly, 4, ExtColor[CLOSESET+j*3][i-1] );
				// top side
				set3dpoly( poly, PX(i-1), PX(i-1)+hlf_hlccapwdth,
						   PY(CELLVALUE(CLOSESET+j*3,i-1))-hlf_hlccapwdth,
						   PY(CELLVALUE(CLOSESET+j*3,i-1)),
						   xdepth_3D, ydepth_3D );
				p->setBrush( QBrush( ExtColor[CLOSESET+j*3][i-1] ) );
				p->setPen( ExtColor[CLOSESET+j*3][i-1] );
				p->drawPolygon( poly );
				p->setBrush( QBrush( ExtColorShd[CLOSESET+j*3][i-1], Qt::NoBrush ) );
				p->setPen( ExtColorShd[CLOSESET+j*3][i-1] );
				p->drawPolygon( poly );
			  }
		  }
	}
}

void KChartEngine::drawPie()
{
  float *val = new float[num_sets];
  QColor *color = new QColor[num_sets];
  QColor *colorShd = new QColor[num_sets];
  for(int i=0; i<num_sets; ++i )
    {
      if( CELLEXISTS(i,params->colPie))
		{
		  val[i] = CELLVALUE(i,params->colPie);
		}
      color[i]=ExtColor[i][0];
      colorShd[i]=ExtColorShd[i][0];
    }
  pie_gif(imagewidth,imageheight,p,params,num_sets,val,color,colorShd);
  delete [] colorShd;
  delete [] color;
  delete [] val;
}

