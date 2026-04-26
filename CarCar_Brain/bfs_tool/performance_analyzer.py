import pandas as pd
import glob
import io
import os

def run_performance_analysis():
    # 自動定位路徑
    script_dir = os.path.dirname(os.path.abspath(__file__))
    data_folder = os.path.join(script_dir, "running_data")
    output_file = os.path.join(script_dir, "Carcar_Performance_Report.xlsx")
    
    all_files = glob.glob(os.path.join(data_folder, "*.txt"))
    if not all_files:
        print(f"在 {data_folder} 找不到任何 .txt 數據檔案。")
        return

    all_dataframes = []
    
    for file_idx, filename in enumerate(all_files):
        with open(filename, 'r', encoding='utf-8', errors='ignore') as f:
            lines = f.readlines()
        
        clean_lines = []
        is_capturing = False
        
        for line in lines:
            if "--- FINAL REPORT ---" in line:
                is_capturing = True
                continue
            if "--- END REPORT ---" in line:
                is_capturing = False
                break 
            if is_capturing and line.strip():
                clean_lines.append(line.strip())
        
        if len(clean_lines) <= 1:
            continue
            
        csv_data = "\n".join(clean_lines)
        
        try:
            df = pd.read_csv(io.StringIO(csv_data))
            
            # 確保數字欄位真的是數字 (過濾掉可能的殘留雜訊)
            numeric_cols = ['Count', 'TotalTime(ms)', 'MaxTime(ms)', 'MinTime(ms)', 'AvgTime(ms)']
            for col in numeric_cols:
                df[col] = pd.to_numeric(df[col], errors='coerce')
            df = df.dropna()
            
            df.insert(0, 'Run_ID', f"Run_{file_idx + 1}")
            all_dataframes.append(df)
            
        except Exception as e:
            print(f"⚠️ 解析檔案 {filename} 時發生錯誤: {e}")

    if not all_dataframes: 
        print("❌ 沒有成功解析出任何有效的數據。")
        return

    # 合併所有跑圖資料
    merged_df = pd.concat(all_dataframes, ignore_index=True)
    
    # 🌟 關鍵修改：利用 sum() 來進行加權計算的準備
    analysis_summary = merged_df.groupby('Action').agg(
        Runs_Count=('Run_ID', 'nunique'),            # 有幾趟跑圖包含這個動作
        Total_Executions=('Count', 'sum'),           # 【分母】總共執行了幾次
        Total_Time_Sum=('TotalTime(ms)', 'sum'),     # 【分子】這些次數總共花了多少時間
        Time_Fluctuation_Std=('AvgTime(ms)', 'std'), # 各次回報的平均時間波動 (標準差)
        Max_Time_Ever=('MaxTime(ms)', 'max'),
        Min_Time_Ever=('MinTime(ms)', 'min')
    ).reset_index()

    # 🌟 計算真正的加權平均：總時間 / 總次數
    analysis_summary['Weighted_AvgTime(ms)'] = analysis_summary['Total_Time_Sum'] / analysis_summary['Total_Executions']

    # 重新排列欄位順序讓報表更好讀，並四捨五入到小數點第二位
    analysis_summary = analysis_summary[[
        'Action', 
        'Runs_Count', 
        'Total_Executions', 
        'Weighted_AvgTime(ms)', 
        'Time_Fluctuation_Std', 
        'Max_Time_Ever', 
        'Min_Time_Ever'
    ]].round(2)

    # 寫入 Excel
    with pd.ExcelWriter(output_file, engine='openpyxl') as writer:
        merged_df.to_excel(writer, sheet_name='All_Runs_Data', index=False)
        analysis_summary.to_excel(writer, sheet_name='Analysis_Summary', index=False)
    
    print(f"✅ 分析完成！精準加權報告已儲存至：{output_file}")

if __name__ == "__main__":
    run_performance_analysis()