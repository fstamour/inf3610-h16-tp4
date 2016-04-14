library ieee;
use ieee.std_logic_1164.all;

entity Sobel_top is

    generic (
	    	AWIDTH : integer := 32 ;     	
	    	DWIDTH : integer := 32     	
		   );

    port (
			SDL0_M_Data : out std_logic_vector(0 to (DWIDTH-1)) ; 		
			SDL0_M_Write : out std_logic ; 		
			SDL0_M_Full : in std_logic ; 		
			SDL0_S_Data : in std_logic_vector(0 to (DWIDTH-1)) ; 		
			SDL0_S_Read : out std_logic ; 		
			SDL0_S_Empty : in std_logic ; 		
			Clk : in std_logic ; 		
			Reset : in std_logic 		
		);

end Sobel_top;

architecture STRUCTURE of Sobel_top is
component Sobel is


    port (
			nResetPort : in std_logic ; 		
			ClockPort : in std_logic ; 		
			fifo_in_0_dout : in STD_LOGIC_VECTOR(31 downto 0) ; 		
			fifo_in_0_empty_n : in std_logic ; 		
			fifo_in_0_read : out std_logic ; 		
			fifo_out_0_din : out STD_LOGIC_VECTOR(31 downto 0) ; 		
			fifo_out_0_full_n : in std_logic ; 		
			fifo_out_0_write : out std_logic 		
		);

end component;

begin
Sobel_instance : Sobel


    port map (
			ClockPort => Clk , 		
			fifo_in_0_dout => SDL0_S_Data , 		
			fifo_in_0_empty_n => "not"(SDL0_S_Empty) , 		
			fifo_in_0_read => SDL0_S_Read , 		
			fifo_out_0_din => SDL0_M_Data , 		
			fifo_out_0_full_n => "not"(SDL0_M_Full) , 		
			fifo_out_0_write => SDL0_M_Write , 		
			nResetPort => Reset 		
		);

end architecture STRUCTURE;