using PlayFab;
using PlayFab.ClientModels;

namespace NetworkingDLL
{
    public static class NetworkSystem
    {
        public static String PlayerId { get; private set; }
        public static String EntityToken { get; private set; }
        public static async Task CreateAccount(string userName, string password, string email)
        {
           var result = await PlayFabClientAPI.RegisterPlayFabUserAsync(new RegisterPlayFabUserRequest
            {
                Email = email,
                Username = userName,
                Password = password,
                RequireBothUsernameAndEmail = true,
            });
            if (result.Error != null)
            {
                Console.WriteLine($"[PlayFab] Register Failed: {result.Error.ErrorMessage}");
                return;
            }

            Console.WriteLine($"[PlayFab] Successfully registered user: {userName}");
            NetworkSystem.PlayerId = result.Result.EntityToken.Entity.Id;
            NetworkSystem.EntityToken = result.Result.EntityToken.EntityToken;
        }

        public static async Task SignIn(string userName, string password)
        {
            var result = await PlayFabClientAPI.LoginWithPlayFabAsync(new LoginWithPlayFabRequest
            {
                Username = userName,
                Password = password,
            });
            if(result.Error != null)
            {
                Console.WriteLine($"[PlayFab] LogIn Failed: {result.Error.ErrorMessage}");
                return;
            }
        }
    }
}
