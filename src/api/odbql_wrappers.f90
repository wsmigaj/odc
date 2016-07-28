

!!!!! THIS FILE HAS BEEN AUTOMATICALLY GENERATED. DO NOT EDIT MANUALLY !!!!!

module odbql_wrappers
  use odbql_binding
  implicit none
contains



!> Helper function to convert C '\0' terminated strings to Fortran strings

    function C_to_F_string(c_string_pointer) result(f_string)
      use, intrinsic :: iso_c_binding, only: c_ptr,c_f_pointer,c_char,c_null_char
      type(c_ptr), intent(in)                       :: c_string_pointer
      character(len=:), allocatable                 :: f_string
      character(kind=c_char), dimension(:), pointer :: char_array_pointer => null()
      character(len=255)                            :: aux_string
      integer                                       :: i,length

      call c_f_pointer(c_string_pointer,char_array_pointer,[255])
      if (.not.associated(char_array_pointer)) then
          allocate(character(len=4)::f_string)
          f_string = "NULL"
          return
      end if
      aux_string = " "
      do i=1,255
        if (char_array_pointer(i)==c_null_char) then
          length=i-1; exit
        end if
        aux_string(i:i)=char_array_pointer(i)
      end do
      allocate(character(len=length)::f_string)
      f_string = aux_string(1:length)
    end function C_to_F_string




!> const char * odbql_errmsg(odbql* db)

    subroutine odbql_errmsg (db,return_value) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: db
     character(len=*),intent(out)         :: return_value

     

     

     return_value = C_to_F_string(odbql_errmsg_c(db))

    end subroutine odbql_errmsg

    

!> const char * odbql_libversion(void)

    subroutine odbql_libversion (return_value) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     
     character(len=*),intent(out)         :: return_value

     

     

     return_value = C_to_F_string(odbql_libversion_c())

    end subroutine odbql_libversion

    

!> int odbql_open(const char *filename, odbql **ppDb)

    function odbql_open (filename,ppDb) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     character(len=*),intent(in)          :: filename
     type(C_PTR)                          :: ppDb
     integer(kind=C_INT)                  :: odbql_open

     character(len=len_trim(filename)+1)  :: filename_tmp

     filename_tmp = filename//achar(0)

     odbql_open = odbql_open_c(filename_tmp,ppDb)

    end function odbql_open

    

!> int odbql_close(odbql* db)

    function odbql_close (db) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: db
     integer(kind=C_INT)                  :: odbql_close

     

     

     odbql_close = odbql_close_c(db)

    end function odbql_close

    

!> int odbql_prepare_v2(odbql *db, const char *zSql, int nByte, odbql_stmt **ppStmt, const char **pzTail)

    function odbql_prepare_v2 (db,zSql,nByte,ppStmt,pzTail) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: db
     character(len=*),intent(in)          :: zSql
     integer(kind=C_INT), VALUE           :: nByte
     type(C_PTR)                          :: ppStmt
     character(len=*),intent(out)         :: pzTail
     integer(kind=C_INT)                  :: odbql_prepare_v2

     character(len=len_trim(zSql)+1)      :: zSql_tmp

     zSql_tmp = zSql//achar(0)

     odbql_prepare_v2 = odbql_prepare_v2_c(db,zSql_tmp,nByte,ppStmt,pzTail)

    end function odbql_prepare_v2

    

!> int odbql_step(odbql_stmt* stmt)

    function odbql_step (stmt) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT)                  :: odbql_step

     

     

     odbql_step = odbql_step_c(stmt)

    end function odbql_step

    

!> int odbql_bind_double(odbql_stmt* stmt, int i, double v)

    function odbql_bind_double (stmt,i,v) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: i
     real(kind=C_DOUBLE), VALUE           :: v
     integer(kind=C_INT)                  :: odbql_bind_double

     

     

     odbql_bind_double = odbql_bind_double_c(stmt,i,v)

    end function odbql_bind_double

    

!> int odbql_bind_int(odbql_stmt* stmt, int i, int v)

    function odbql_bind_int (stmt,i,v) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: i
     integer(kind=C_INT), VALUE           :: v
     integer(kind=C_INT)                  :: odbql_bind_int

     

     

     odbql_bind_int = odbql_bind_int_c(stmt,i,v)

    end function odbql_bind_int

    

!> int odbql_bind_null(odbql_stmt* stmt, int i)

    function odbql_bind_null (stmt,i) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: i
     integer(kind=C_INT)                  :: odbql_bind_null

     

     

     odbql_bind_null = odbql_bind_null_c(stmt,i)

    end function odbql_bind_null

    

!> int odbql_bind_text(odbql_stmt* stmt, int i, const char* s, int n, void(*d)(void*))

    function odbql_bind_text (stmt,i,s,n,d) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: i
     character(len=*),intent(in)          :: s
     integer(kind=C_INT), VALUE           :: n
     type(C_PTR), VALUE                   :: d
     integer(kind=C_INT)                  :: odbql_bind_text

     character(len=len_trim(s)+1)         :: s_tmp

     s_tmp = s//achar(0)

     odbql_bind_text = odbql_bind_text_c(stmt,i,s_tmp,n,d)

    end function odbql_bind_text

    

!> const unsigned char *odbql_column_text(odbql_stmt* stmt, int column)

    subroutine odbql_column_text (stmt,column,return_value) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: column
     character(len=*),intent(out)         :: return_value

     

     

     return_value = C_to_F_string(odbql_column_text_c(stmt,column))

    end subroutine odbql_column_text

    

!> int odbql_finalize(odbql_stmt *stmt)

    function odbql_finalize (stmt) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT)                  :: odbql_finalize

     

     

     odbql_finalize = odbql_finalize_c(stmt)

    end function odbql_finalize

    

!> const char *odbql_column_name(odbql_stmt* stmt, int iCol)

    subroutine odbql_column_name (stmt,iCol,return_value) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: iCol
     character(len=*),intent(out)         :: return_value

     

     

     return_value = C_to_F_string(odbql_column_name_c(stmt,iCol))

    end subroutine odbql_column_name

    

!> int odbql_column_type(odbql_stmt* stmt, int iCol)

    function odbql_column_type (stmt,iCol) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT), VALUE           :: iCol
     integer(kind=C_INT)                  :: odbql_column_type

     

     

     odbql_column_type = odbql_column_type_c(stmt,iCol)

    end function odbql_column_type

    

!> int odbql_column_count(odbql_stmt *stmt)

    function odbql_column_count (stmt) 
     use odbql_binding
     use, intrinsic                       :: iso_c_binding
     type(C_PTR), VALUE                   :: stmt
     integer(kind=C_INT)                  :: odbql_column_count

     

     

     odbql_column_count = odbql_column_count_c(stmt)

    end function odbql_column_count

    

end module odbql_wrappers
