function fast_read_csv(file_path::String)
	col_num = ccall((:get_file_cols, "./fast_read.so"), Cint, (Cstring,), file_path)
	row_num = ccall((:get_file_rows, "./fast_read.so"), Cint, (Cstring,), file_path)
	address = ccall((:get_file, "./fast_read.so"), Ptr{Cdouble}, (Cstring, Cint, Cint), file_path, row_num, col_num)
	data = unsafe_wrap(Array, address, (Int64(col_num), Int64(row_num)))'
	return data
end

@time A = fast_read_csv("./BTCUSDT-trades-2021-06-small.csv")
