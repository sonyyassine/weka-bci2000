/* (C) 2000-2008, BCI2000 Project
/* http://www.bci2000.org
/*/
/********************************************************************
        BCIOutput contains the output routines for the
        BCITime application
*********************************************************************/

#include <stdio.h>
// FILE *bcio;

#include "BCIOutput.h"
#include "BCIInput.h"
#include <math.h>
#include <string.h>

BCIOutput::BCIOutput()
: mSidelobeSuppression( WindowingFunction::None )
{
     //  bcio= fopen("c:/shared/misc/BCIO2.asc","w+");
     //  fprintf(bcio,"Opening BCIO.asc \n");
}
BCIOutput::~BCIOutput()
{
     //  fclose(bcio);
}

void __fastcall BCIOutput::Config( char *outfile, int sr, float start, int order, int stat )
{
        int i,j,k;

        Hz= sr;
        memptr->setHz( Hz );

        fstart= start;

        xyorder= order;


        if( order == 2 )
        {
                strcpy(inf,"VOID");
               //  achans= 0;
               //  buf_lth= 0;
                strcpy( outf_base, outfile );
        }
        else
        {
                if( (fileptr= fopen(outfile,"w+")) == NULL )
                {
                       // Application->MessageBox("Error opening output file", "Error", MB_OK);
                        return;
                }
         }

        statistics=  stat;

        for(i=0;i<NGROUPS;i++)
        {
                for(j=0;j<MAXCHANS;j++)
                {
                        for(k=0;k<MAXPOINTS;k++)
                        {
                                point[i][j][k]= 0;
                                sspoint[i][j][k]= 0;
                                xypoint[i][j][k]= 0;
                                n[i][j][k]= 0;
                                spoint[i][j][k]= 0;
                         }
                 }
                 for(j=0;j<MAXSTATES;j++)
                 {
                        for(k=0;k<MAXPOINTS;k++)
                        {
                                stateval[i][j][k]= 0;
                                nsv[i][j][k]= 0;
                                sstate[i][j][k]= 0;
                                xystate[i][j][k]= 0;
                        }
                }
        }
        maxgroup= 0;
        maxchan= 0;
        maxpoint= 0;
        maxstate= 0;
}

void __fastcall BCIOutput::CloseFiles()
{
        if( fileptr ) fclose( fileptr );

}

void __fastcall BCIOutput::ClearVals( void )
{
        int i,j,k;

        for(i=0;i<maxgroup+1;i++)
        {
                for(j=0;j<maxchan+1;j++)
                {
                        for(k=0;k<maxpoint+1;k++)
                        {
                                point[i][j][k]= 0;
                                sspoint[i][j][k]= 0;
                                xypoint[i][j][k]= 0;
                                n[i][j][k]= 0;
                        }
                 }
                 for(j=0;j<maxstate+1;j++)
                 {
                        for(k=0;k<maxpoint+1;k++)
                        {
                                stateval[i][j][k]= 0;
                                nsv[i][j][k]= 0;
                                sstate[i][j][k]= 0;
                                xystate[i][j][k]= 0;
                        }
                }
        }
        maxgroup= 0;
        maxchan= 0;
        maxpoint= 0;
        maxstate= 0;
}

void __fastcall BCIOutput::AddPoint( int group, int chan, int time, float val )
{
        if( group > NGROUPS ) return;
        if( chan  > MAXCHANS ) return;
        if( time  > MAXPOINTS ) return;

        if( group > maxgroup ) maxgroup= group;
        if( chan > maxchan ) maxchan= chan;
        if( time >= maxpoint ) maxpoint= time+1;                  // ##

        point[group][chan][time]+= val;
        n[group][chan][time]++;

        if( statistics > 0 )
        {
                 sspoint[group][chan][time]+= val*val;
                 xypoint[group][chan][time]+= val*group;
        }
}

void BCIOutput::setWindow( int wuse, int nwin, int wblock, int wlength, const WindowingFunction& inSidelobeSuppression )
{
      wintype= wuse;                    // type of windowing  0 is all data in one window
      wblocksz= wblock;                 // window block size
      winnum= nwin;                     // number of blocks per window
      winlength= wlength;               // total data length = wblocksz * winnum
      mSidelobeSuppression = inSidelobeSuppression;

      // compute things??
}


void __fastcall BCIOutput::AddSpcPoint( int group, int chan, int time, float val )
{
        if( group > NGROUPS ) return;
        if( chan  > MAXCHANS ) return;
        if( time  > MAXPOINTS ) return;

  //      if( group > maxgroup ) maxgroup= group;
  //      if( chan > maxchan )   maxchan= chan;
  //      if( time >= maxpoint ) maxpoint= time+1;

        spoint[group][chan][time]= val;

        if( sn[group][chan] < time ) sn[group][chan]= time;
}

void __fastcall BCIOutput::DumpSpc( void )
{
        int i,j,k;
        int lth;
        int ret;
        int pts;
        int index;
        float vect[MAXPOINTS];
        float pwr[MAXBINS];
        float spwr[MAXBINS];
        int npwr;
    //    int totpts;             // total points
        int rempts;             // points remaining
        int kstart;
        int kend;

        for(i=0;i<NGROUPS;i++)
        {
                for(j=0;j<MAXCHANS;j++)
                {
                        if( sn[i][j] > 0 )
                        {

//*************** do windowing here **************

                           rempts= sn[i][j] + 1 ;

                           if( wintype == 0 )       // only 1 window in this case
                           {
                                winlength= rempts;
                                wblocksz= winlength;
                           }

                           for(k=0;k<MAXBINS;k++)
                           {
                                spwr[k]= 0;
                           }
                           npwr= 0;

                           while( rempts >= winlength )
                           {
                                kstart= rempts-winlength;
                                kend= rempts;
                                rempts-= wblocksz;

                                index= 0;
                                for(k=kstart;k<kend;k++)        //  for(k=0;k<sn[i][j];k++)
                                {
                                        vect[index]= spoint[i][j][k];
                                        vect[ index ] *= mSidelobeSuppression.Value( float( index ) / float( winlength ) ); 
                                        index++;
                                }


                                lth= memptr->setData( winlength, vect );
                                ret= memptr->get_mem();
                                pts= memptr->get_pwr( pwr );

                                if( pts > 0 )
                                {
                                        for(k=0;k<pts;k++)
                                                spwr[k]+= pwr[k];
                                        npwr++;
                                }
                            }
                            if( npwr > 0 )
                            {

                                for(k=0;k<pts;k++)
                                        AddPoint( i, j, k, spwr[k]/npwr );
                            }
//*******end windowing ***************************
                        }

                }
         }

         for(i=0;i<NGROUPS;i++)
         {
                for(j=0;j<MAXCHANS;j++)
                {
                        sn[i][j]= 0;

                        for(k=0;k<MAXPOINTS;k++)
                        {
                                spoint[i][j][k]= 0.0;
                        }
                }
         }
}

void __fastcall BCIOutput::AddStateVal( int group, int state, int time, float val )
{
        if( group > NGROUPS ) return;
        if( state  > MAXSTATES ) return;
        if( time  > MAXPOINTS ) return;

        if( group > maxgroup ) maxgroup= group;
        if( state > maxstate ) maxstate= state;
        if( time >= maxpoint ) maxpoint= time+1;

        stateval[group][state][time]+= val;
        nsv[group][state][time]++;

        if( statistics > 0 )
        {
                sstate[group][state][time]+= val * val;
                xystate[group][state][time]+= val*group;
        }
}


double __fastcall BCIOutput::GetLr( double *t, double *ss, double *xy, int *n, int ntarg )
{
        int i;
        double r,nn,sumy;
        double sumx,ssx,ssy,sxy;
        double cx,cy,cxy;
        double den;
        double x;

        sumy= 0;
        sumx= 0;
        ssx= 0;
        ssy= 0;
        sxy= 0;
        nn= 0;

        for( i=0;i<ntarg;i++)
        {
                nn+= n[i];

                x= (float)(i+1);
                sumy+= t[i];
                sumx+= x*(float)n[i];

                ssx+= x*x*(float)n[i];
                ssy+= ss[i];
                sxy+= xy[i];
        }

        cxy=    sxy - ( sumx * sumy / nn );
        cx=     ssx - ( sumx * sumx / nn );
        cy=     ssy - ( sumy * sumy / nn );

        den= cx * cy;

        if( den > 0 )
                r= cxy / sqrt( cx * cy );   
        else
                r= -3.0;        

        return(r);
}




void __fastcall BCIOutput::PrintVals( int print_flag )
{
        int i,j,k;
        double r;
        double sum[NGROUPS], ss[NGROUPS], sxy[NGROUPS];
        int nn[NGROUPS];
        char outf[MAXOUT];
        char suf[16];
        int feedflag;
        int ncol;
        int nchan;

        switch(xyorder)
        {
                case 0:                                 // chan x time  e.g. Sigmaplot
                        for(k=0;k<maxpoint;k++)
                        {
                                if( memflag )
                                        fprintf(fileptr,"%8.2f ",memptr->bstart + k*memptr->bandwidth);
                                else
                                        fprintf(fileptr,"%8.2f",fstart + (float)(1000 * k / Hz) );

                                for(j=0;j<maxchan+1;j++)               // + 1 ##
                                {

                                        for(i=1;i<maxgroup+1;i++)      // + 1 ##
                                        {
                                                if( n[i][j][k] > 0 )
                                                {
                                                        fprintf(fileptr," %9.3f",point[i][j][k]/n[i][j][k]);
                                                }
                                                if( statistics > 0 )
                                                {
                                                        sum[i-1]= point[i][j][k];
                                                        ss[i-1]=  sspoint[i][j][k];
                                                        sxy[i-1]= xypoint[i][j][k];
                                                        nn[i-1]=  n[i][j][k];
                                                }
                                        }

                                        if( statistics == 1 )
                                        {
                                                r= GetLr( sum, ss, sxy, nn, maxgroup);
                                                fprintf(fileptr,"    %8.4f",r*r);
                                        }
                                        else if( statistics == 2 )
                                        {
                                                r= GetLr( sum, ss, sxy, nn, maxgroup);
                                                fprintf(fileptr,"    %8.4f",r);
                                        }
                                }
                                for(j=0;j<maxstate+1;j++)
                                {
                                        for(i=1;i<maxgroup+1;i++)
                                        {
                                                if( nsv[i][j][k] > 0 )
                                                {
                                                        fprintf(fileptr," %9.3f",stateval[i][j][k]/nsv[i][j][k]);
                                                }
                                        }
                                }
                                fprintf(fileptr,"\n");
                        }
                        break;

                case 1:                             // time x chan  e.g. SPSS
                        for(i=1;i<maxgroup+1;i++)
                        {
                                for(j=0;j<maxstate+1;j++)
                                {
                                        feedflag= 0;
                                        for(k=0;k<maxpoint;k= k+decimate)
                                        {
                                                if( nsv[i][j][k] > 0 )
                                                {
                                                        fprintf(fileptr," %9.3f",stateval[i][j][k]/nsv[i][j][k]);
                                                        feedflag++;
                                                }
                                        }
                                        if( feedflag > 0 )
                                        {
                                                fprintf(fileptr,"\n");
                                                feedflag= 0;
                                        }        
                                }
                                for(j=0;j<maxchan+1;j++)
                                {
                                      if( n[i][j][0] > 0 )
                                      {
                                        fprintf(fileptr,"%3d %3d %3d",print_flag,i,j+1);

                                        for(k=0;k<maxpoint;k= k+decimate)
                                        {
                                                if( n[i][j][k] > 0 )
                                                        fprintf(fileptr," %9.3f",point[i][j][k]/n[i][j][k]);
                                        }
                                        fprintf(fileptr,"\n");
                                      }
                                }
                        }
                        break;
                case 2:                           // Topographies
                        for(k=0;k<ntimes;k++)
                        {

//   fprintf(bcio,"time_list[%2d]= %4d  value_list[%2d]= %4d \n",k,time_list[k],k,value_list[k]);

                                strcpy( outf, outf_base );
                                strcat( outf,".");
                                itoa( time_list[k],suf,10);
                                strcat( outf, suf );

                                if( (fileptr= fopen(outf,"w+")) == NULL )
                                {
                                        return;
                                }

                                ncol= 0;

                                for(i=1;i<maxgroup+1;i++)              // reverse order- need at least 1 group value
                                {
                                        for(j=0;j<maxchan+1;j++)
                                        {
                                                if( n[i][j][value_list[k]] > 0 )
                                                {
                                                        ncol++;
                                                        goto nxt;
                                                }
                                        }
                                        nxt:
                                }

                                nchan= 0;

                                 for(j=0;j<maxchan+1;j++)
                                 {
                                        for(i=1;i<maxgroup+1;i++)              // need at least 1 chan value
                                        {

                                                if( n[i][j][value_list[k]] > 0 )
                                                {
                                                        nchan++;
                                                        goto nxt2;
                                                }
                                        }
                                        nxt2:
                                }

                                print_hdr(fileptr, suf, ncol, nchan );

                                for(j=0;j<maxchan+1;j++)
                                {
                                        for(i=1;i<maxgroup+1;i++)
                                        {
                                                if( n[i][j][value_list[k]] > 0 )
                                                {
                                                        fprintf(fileptr," %10.3f",point[i][j][value_list[k]]/n[i][j][value_list[k]]);
                                                }
                                                if( statistics > 0 )
                                                {
                                                        sum[i-1]= point[i][j][value_list[k]];
                                                        ss[i-1]=  sspoint[i][j][value_list[k]];
                                                        sxy[i-1]= xypoint[i][j][value_list[k]];
                                                        nn[i-1]=  n[i][j][value_list[k]];
                                                }
                                        }
                                        if( statistics > 0 )
                                        {
                                                r= GetLr( sum, ss, sxy, nn, maxgroup);

                                                if( ncol < 4 )
                                                        fprintf(fileptr,"    %8.4f  %8.4f",r*r, 1+r);      // add 1 to r for HzPlot
                                                else
                                                        fprintf(fileptr,"    %8.4f  %8.4f  %8.4f  %8.4f",r*r,r*r,1+r,1+r);
                                        }
                                        fprintf(fileptr,"\n");
                                }
                                fclose(fileptr);
                                fileptr = NULL;
                        }
                        break;
         }
}

void __fastcall BCIOutput::print_hdr(FILE *otf, char *time, int cols, int chans)
{
        char cur_out[80];
        char ext[15];
        float hzctr;
        float bandwidth;
	char chz[12];
	char type[2][16];
        int i;


	strcpy(type[0],"RSQ");
	strcpy(type[1],"VOID");

        fprintf(otf,"TOPO %s (%d channels) at %s \n",inf,chans,time);
	fprintf(otf,"Order: %d\n", 7734);
	fprintf(otf,"Zero Padding:  1\n");
	fprintf(otf,"Target filter: ");
        fprintf(otf,"Other\n");
        fprintf(otf,"Use Pretrial Data: OFF\n");
        fprintf(otf,"Detrend data: OFF\n");
        fprintf(otf,"Scale data: ON; Factor: 0.12500 \n");
        fprintf(otf,"Slide data: OFF\n");
        fprintf(otf,"Hamming window: OFF\n");
        fprintf(otf,"Remove grand mean: OFF\n");

        fprintf(otf,"%2d %3d %s ",cols,chans,time);

        for(i=0;i<cols;i++)
                fprintf(otf,"%2d ",i+1);
                
        fprintf(otf,"\n");
 }



