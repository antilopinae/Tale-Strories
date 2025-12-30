#include "Kismet/BlueprintFunctionLibrary.h"
#include "lib_ue.hpp"
#include "TaleStoriesBPLibrary.generated.h"

UCLASS()
class TALESTORIES_API UTaleStoriesBPLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, Category = "ThirdPartyLib")
	static int32 TestAddNumbers(int32 A, int32 B)
	{
		return AddNumbers(A, B);
	}
};
