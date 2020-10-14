set(Models_Tests
	"block/Test_Block.cpp"
	"block/Test_Header.cpp"
	"crypto/Test_BigInteger.cpp"
	"tx/Test_Input.cpp"
	"tx/Test_Kernel.cpp"
	"tx/Test_Output.cpp"
	"tx/Test_OutputId.cpp"
	"tx/Test_PegInCoin.cpp"
	"tx/Test_PegOutCoin.cpp"
	"tx/Test_Transaction.cpp"
	"tx/Test_TxBody.cpp"
	"tx/Test_UTXO.cpp"
)

list_prepend(Models_Tests ${CMAKE_CURRENT_LIST_DIR}/)

list(APPEND test_sources ${Models_Tests})
set(test_sources ${test_sources} PARENT_SCOPE)