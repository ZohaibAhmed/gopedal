class CreateBikes < ActiveRecord::Migration
  def change
    create_table :bikes do |t|
      t.boolean :lock

      t.timestamps
    end
  end
end
