import yfinance as yf

# filename of your text file
filename = "../symbols/all_tickers.txt"

# Read tickers into a list
with open(filename, "r") as file:
    tickers = [line.strip() for line in file]

# print(tickers)


# # Create a ticker object
# apple = yf.Ticker("AAPL")

for idx, ticker in enumerate(tickers):
    
   # Create a ticker object 
    yfTicker = yf.Ticker(ticker)
    
    # Get historical market data
    hist = yfTicker.history(period="5y")  # last 5 years

    # print(f'idx = {idx}\nhist.head() = {hist.head()}')  # display first few rows
    file_name = f'./HistoricData5Years/{ticker}_stocks_data.csv'
    
    hist.to_csv(file_name)
    print(f'finished saving data for {ticker}')

if __name__ == "__main__":
    print(' running from top-level module ')