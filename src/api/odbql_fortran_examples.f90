! (C) Copyright 1996-2012 ECMWF.
! 
! This software is licensed under the terms of the Apache Licence Version 2.0
! which can be obtained at http://www.apache.org/licenses/LICENSE-2.0. 
! In applying this licence, ECMWF does not waive the privileges and immunities 
! granted to it by virtue of its status as an intergovernmental organisation nor
! does it submit to any jurisdiction.
!
! Piotr Kuchta  July 2016


program example_fortran_api
  use odbql_wrappers
  implicit none
  character(len=255)                           :: version

  call odbql_libversion(version)
  write(0,*) "This program is linked to ODB API version: ", version

  call odbql_fortran_example()

contains

subroutine odbql_fortran_example
 implicit none
 type(C_PTR)                                   :: db, stmt
 integer(kind=C_INT)                           :: rc, number_of_columns, i
 character(len=30)                             :: val, column_name
 character(len=1000)                           :: unparsed_sql
 real(kind=C_DOUBLE)                           :: v


!!!! Write to a file with INSERT

 rc = odbql_open("CREATE TABLE foo AS (x INTEGER, y INTEGER, v REAL) ON 'fort.odb';", db)
 rc = odbql_prepare_v2(db, "INSERT INTO foo (x,y,v) VALUES (?,?,?);", -1, stmt, unparsed_sql)

 do i=0,3 - 1
    rc = odbql_bind_int(stmt, 0, 1 * (i + 1));
    rc = odbql_bind_int(stmt, 1, 10 * (i + 1));
    v = 0.1 * (i + 1)
    rc = odbql_bind_double(stmt, 2, v);
    rc = odbql_step(stmt);
 enddo
 rc = odbql_finalize(stmt);
 rc = odbql_close(db)

!! Print first row of query result set

! Associate table with a file name
 rc = odbql_open("CREATE TABLE foo ON 'fort.odb';", db)
! Retrieve some data from MARS:
! rc = odbql_open("CREATE TABLE foo ON 'mars://RETRIEVE,CLASS=OD,TYPE=MFB,STREAM=OPER,EXPVER=0001,DATE=20160720,TIME=1200,DATABASE=marsod';", db)
 rc = odbql_prepare_v2(db, "SELECT * FROM foo;", -1, stmt, unparsed_sql)
 number_of_columns = odbql_column_count(stmt)
 write(0,*) "Number of columns: ", number_of_columns 

 rc = odbql_step(stmt)

 do i=0,number_of_columns - 1
     call odbql_column_name(stmt, i, column_name)
     call odbql_column_text(stmt, i, val)
     write(0,*) i, ' ', column_name, ': ', val
 enddo

 rc = odbql_finalize(stmt)
 rc = odbql_close(db)

end subroutine odbql_fortran_example

end program example_fortran_api
